#define IMGUI_DEFINE_MATH_OPERATORS  // before any include of imgui.h (runner_params.h includes it)
#include "hello_imgui/internal/touch_scroll.h"
#include "hello_imgui/hello_imgui.h"  // RequestRefresh()

#include "imgui.h"
#include "imgui_internal.h"

#include <cmath>
#include <vector>

// The swipe, and the delayed press of the mobile toolkits, as a layer above the backends and below the widgets.
// It runs right after ImGui::NewFrame(), before any widget:
// - a touch press is claimed with a sentinel active id. ImGui hovers no item while another id is active, so no widget
//   sees the press, and the window does not move. (The ownership part of ocornut's snippet in
//   https://github.com/ocornut/imgui/issues/3379, taken before the widgets instead of after them.)
// - the finger moves past a slop: a swipe. The window to scroll is chosen once, as ImGui's wheel would (the pressed
//   one, or the first parent that can scroll on the axis), and follows the finger from then on, even when the finger
//   leaves it. On the release, the speed of the finger becomes an inertia that decays.
// - the finger lifts before the slop: a tap. The press and the release are replayed through the input queue, and the
//   widget under the finger gets a normal click, two frames later.
// - the finger stays still past a hold delay: the press is handed over, by replaying a release and a press while the
//   finger is down. The widget under it activates, and the real finger drives its drag (a slider, a text selection).
// A widget that takes the active id itself ends the swipe (the layer steps aside). A widget that called
// SetItemTakesTouchDrags() at the last frame gets a press on it at once: on a touch screen nothing is hovered before
// a press, so the layer reads the rectangles that the widgets noted at the last frame.
// On a touch screen, there is no pointer between two touches: when a finger lifts, the mouse position becomes
// invalid (as when a mouse leaves the window), so that nothing is hovered while the content coasts, nor after a tap.
// A finger still for half a second arms a long press, shown by a steady ring around it: when the finger lifts, it is a
// right click (the context menus), as Windows' press-and-hold. Its release reaches no widget (no pointer on that
// frame): a long press on a button is not a click. A move after the ring cancels it: the widget under the finger keeps
// the press it got at the hold (a slider that drags after a pause, a text selection that grows).
// Past the end of the scroll, the content follows the finger with a growing resistance (the rubber band), and an
// inertia that reaches the end overshoots; both spring back. ImGui clamps the scroll at each Begin(), so the content
// past its end is drawn by moving its vertices after ImGui::Render() (ApplyTouchOverscroll), inside the window's
// clip rect: its background, border and scrollbars stay.
namespace HelloImGui
{
namespace
{
    // Tunables. The slop is in font sizes, so that it follows the DPI and the font scale.
    constexpr float kSlopFontSizes = 0.5f;      // a press that moved less than this is a tap, not a swipe (a mouse)
    constexpr float kSlopFontSizesTouch = 1.f;  // the same for a finger, which jitters more, and is a finger wide
    constexpr float kHoldRippleSeconds = 0.35f; // the ring drawn around the finger when the hold hands it the press
    constexpr float kDoubleTapSeconds = 0.4f;   // two taps closer than this in time, and than kDoubleTapFontSizes...
    constexpr float kDoubleTapFontSizes = 1.5f; // ...in distance, are a double click (ImGui's 0.3 s and 6 px suit a mouse)
    constexpr float kHoldSeconds = 0.15f;       // a finger still for this long hands the press to the widget under it (iOS: 150 ms)
    constexpr float kLongPressSeconds = 0.5f;   // a finger still for this long arms a right click, at the lift (iOS: about 500 ms)
    constexpr float kInertiaDecay = 2.f;        // speed *= exp(-decay * dt) after the release (iOS: 0.998 per ms)
    constexpr float kInertiaMinSpeedEm = 3.f;   // font sizes per second: below this, a release starts no inertia, and the inertia ends
    constexpr float kFlickWindow = 0.05f;       // s: the lift speed is the finger's motion over this long before the lift
    constexpr int kFlickSamples = 16;           // enough for the window at 240 fps
    constexpr float kRubberBandResistance = 0.55f;  // iOS: a drag of d past the end moves the content by (1 - 1 / (d * c / size + 1)) * size
    constexpr float kThinScrollbarFontSizes = 0.3f;  // the scroll bars' width on a touch screen (iOS: about 3 pt)
    constexpr float kBounceOmega = 12.f;        // 1/s: the content past its end springs back as a critically damped spring (settled in about 0.4 s)

    struct Sample
    {
        float time;
        ImVec2 pos;
    };

    struct State
    {
        ImGuiContext* context = nullptr;  // the state belongs to one context (its windows): a new one starts afresh
        int frameCount = -1;              // (a new context may reuse the address of a destroyed one: its frames restart)
        bool owning = false;            // the sentinel holds the active id: a press, not released nor handed over yet
        bool swiping = false;           // the finger moved past the slop
        bool parked = false;            // the sentinel holds the active id for a pinch: nothing until the fingers lift
        ImGuiWindow* window = nullptr;  // the pressed window, then the one that scrolls
        ImGuiAxis axis = ImGuiAxis_None;
        ImVec2 pressPos;
        Sample samples[kFlickSamples];  // the finger's recent positions, a ring: the lift speed comes from them
        int nbSamples = 0, nextSample = 0;
        ImVec2 inertia;                 // px/s, after the release
        float pull = 0.f;               // px: how far the finger dragged past the end of the scroll (the rubber band)
        float overscroll = 0.f;         // px: the content is drawn this far past its end, along the axis (positive: toward the start)
        float overscrollSpeed = 0.f;    // px/s, while it springs back
        int replayedPresses = 0;        // presses queued by the layer, which it must not claim
        bool watchingLongPress = false; // a touch press, still so far: a long press when it stays
        bool longPressArmed = false;    // it stayed: a right click when it lifts, unless it moves (a ring shows it)
        double lastReplayTime = -1e9;   // the previous replayed press: the next one pairs with it (a double tap)
        double rippleTime = -1e9;       // when the hold handed the press over: a ring around the finger, briefly
        ImVec2 ripplePos;
        ImVec2 lastReplayPos;
        int lastReplayCount = 0;
        float pressTime = 0.f;
        struct DragTaker
        {
            ImGuiID window;  // the widget's window
            ImRect rect;     // its visible part, on the screen
            bool longPressIsRightClick;
        };
        std::vector<DragTaker> takers, previousTakers;  // SetItemTakesTouchDrags(): this frame's, the last frame's
    };
    State gState;

    ImGuiID SentinelId() { return ImHashStr("##HelloImGui_TouchScroll"); }

    float Slop(const ImGuiContext& g)
    {
        return g.FontSize * (g.IO.MouseSource == ImGuiMouseSource_TouchScreen ? kSlopFontSizesTouch : kSlopFontSizes);
    }

    // The feedback of the hold: a ring that grows and fades around the finger, so that the user knows the widget
    // under it has the press, and the drag can start
    void DrawHoldRipple(const State& s)
    {
        ImGuiContext& g = *GImGui;
        float t = (float)(g.Time - s.rippleTime) / kHoldRippleSeconds;
        if (t < 0.f || t > 1.f || !ImGui::IsMousePosValid(&s.ripplePos))
            return;
        float radius = g.FontSize * (1.f + 1.5f * t);
        ImU32 col = ImGui::GetColorU32(ImGuiCol_Text, 0.9f * (1.f - t));
        ImGui::GetForegroundDrawList()->AddCircle(s.ripplePos, radius, col, 0, g.FontSize * 0.25f);
    }

    // The long press is armed: a steady ring around the finger, until it lifts (a right click) or moves (cancelled)
    void DrawLongPressRing(const State& s)
    {
        ImGuiContext& g = *GImGui;
        if (!s.longPressArmed || !ImGui::IsMousePosValid())
            return;
        ImU32 col = ImGui::GetColorU32(ImGuiCol_Text, 0.6f);
        ImGui::GetForegroundDrawList()->AddCircle(g.IO.MousePos, g.FontSize * 1.8f, col, 0, g.FontSize * 0.2f);
    }

    // The axis of a swipe: the dominant direction of the finger (a swipe scrolls one axis, like the wheel)
    ImGuiAxis SwipeAxis(ImVec2 fromPress)
    {
        return (ImFabs(fromPress.x) > ImFabs(fromPress.y)) ? ImGuiAxis_X : ImGuiAxis_Y;
    }

    // The window to scroll on this axis, as ImGui's wheel chooses it (FindBestWheelingWindow): the pressed one, or
    // the first parent that can scroll that way. A child at the end of its scroll keeps the swipe, like the wheel.
    ImGuiWindow* ScrollTarget(ImGuiWindow* w, ImGuiAxis axis)
    {
        for (; w->Flags & ImGuiWindowFlags_ChildWindow; w = w->ParentWindow)
        {
            bool canScroll = (axis == ImGuiAxis_X) ? (w->ScrollMax.x != 0.f) : (w->ScrollMax.y != 0.f);
            bool inputsDisabled = (w->Flags & ImGuiWindowFlags_NoScrollWithMouse) && !(w->Flags & ImGuiWindowFlags_NoMouseInputs);
            if (canScroll && !inputsDisabled)
                break;
        }
        return w;
    }

    // The widget under the press that asked for the drags at the last frame (SetItemTakesTouchDrags), if any: in the
    // pressed window, or in a window that holds it as a child (a plot drawn in a child window of its own)
    const State::DragTaker* FindDragTaker(const State& s, const ImGuiWindow* w, ImVec2 pos)
    {
        for (const State::DragTaker& t : s.previousTakers)
        {
            if (!t.rect.Contains(pos))
                continue;
            for (const ImGuiWindow* x = w; x; x = (x->Flags & ImGuiWindowFlags_ChildWindow) ? x->ParentWindow : nullptr)
                if (x->ID == t.window)
                    return &t;
        }
        return nullptr;
    }

    // With a finger, a window's scroll bars are indicators: a press on one is a swipe, as on the content (dragging
    // the thumb moved the content against the finger, and faster than it). A hold still hands it the thumb.
    bool OnScrollbar(ImGuiWindow* w, ImVec2 pos)
    {
        return (w->ScrollbarY && ImGui::GetWindowScrollbarRect(w, ImGuiAxis_Y).Contains(pos))
            || (w->ScrollbarX && ImGui::GetWindowScrollbarRect(w, ImGuiAxis_X).Contains(pos));
    }

    // Whether the window, or a parent, can scroll at all: when nothing can, a press has nothing to pre-empt
    bool CanScrollSomewhere(const ImGuiWindow* w)
    {
        for (;; w = w->ParentWindow)
        {
            bool canScroll = (w->ScrollMax.x != 0.f || w->ScrollMax.y != 0.f) && !(w->Flags & ImGuiWindowFlags_NoScrollWithMouse);
            if (canScroll)
                return true;
            if (!(w->Flags & ImGuiWindowFlags_ChildWindow))
                return false;
        }
    }

    float Along(ImVec2 v, ImGuiAxis axis) { return axis == ImGuiAxis_X ? v.x : v.y; }

    // Scrolls the window by the finger's motion (the content follows the finger). Applied by its next Begin(),
    // which is later in this frame. Returns the part of the motion past the end of the scroll (zero when all of it
    // scrolls).
    float ScrollBy(ImGuiWindow* w, ImGuiAxis axis, float motion)
    {
        float scroll = Along(w->Scroll, axis), scrollMax = Along(w->ScrollMax, axis);
        float room = (motion > 0.f) ? scroll : scrollMax - scroll;  // how far the content can still go that way
        float absorbed = ImMin(ImFabs(motion), room);
        if (axis == ImGuiAxis_X)
            ImGui::SetScrollX(w, scroll - motion);
        else
            ImGui::SetScrollY(w, scroll - motion);
        return motion - ((motion > 0.f) ? absorbed : -absorbed);
    }

    // The rubber band of iOS: the content follows a finger past the end, by less and less, never past the window
    float RubberBand(float pull, const ImGuiWindow* w, ImGuiAxis axis)
    {
        float size = (axis == ImGuiAxis_X) ? w->InnerRect.GetWidth() : w->InnerRect.GetHeight();
        if (pull == 0.f || size <= 0.f)
            return 0.f;
        float d = (1.f - 1.f / (ImFabs(pull) * kRubberBandResistance / size + 1.f)) * size;
        return (pull > 0.f) ? d : -d;
    }

    void AddSample(State& s, float time, ImVec2 pos)
    {
        s.samples[s.nextSample] = {time, pos};
        s.nextSample = (s.nextSample + 1) % kFlickSamples;
        s.nbSamples = ImMin(s.nbSamples + 1, kFlickSamples);
    }

    // The speed of the finger at the lift: its motion over the last kFlickWindow seconds (a quick flick accelerates
    // until the finger leaves: an average over a longer time would lag behind it). A finger that paused before the
    // lift gives zero.
    ImVec2 LiftSpeed(const State& s, float now, ImVec2 pos)
    {
        const Sample* oldest = nullptr;
        for (int i = 0; i < s.nbSamples; ++i)
        {
            const Sample& sample = s.samples[i];
            if (sample.time <= now - kFlickWindow && (oldest == nullptr || sample.time > oldest->time))
                oldest = &sample;  // the most recent sample at or before the window
        }
        if (oldest == nullptr)  // the press is younger than the window: all of it
            for (int i = 0; i < s.nbSamples; ++i)
                if (oldest == nullptr || s.samples[i].time < oldest->time)
                    oldest = &s.samples[i];
        float span = oldest ? now - oldest->time : 0.f;
        return (span > 0.f) ? (pos - oldest->pos) / span : ImVec2(0.f, 0.f);
    }

    void EndPress(State& s)
    {
        s.owning = false;
        s.swiping = false;
        s.parked = false;
        s.pull = 0.f;  // a content pulled past its end springs back
    }

    // A replayed event. The test engine erases, each frame, the queued events it did not add itself (the backend's):
    // these ones are ImGui's own, not the backend's, so they are marked as the engine marks its own.
    void MarkReplayed(int sizeBefore)
    {
        ImGuiContext& g = *GImGui;
        if (g.InputEventsQueue.Size > sizeBefore)
            g.InputEventsQueue.back().AddedByTestEngine = true;
    }
    void AddReplayedButtonEvent(ImGuiIO& io, bool down, ImGuiMouseButton button = ImGuiMouseButton_Left)
    {
        int sizeBefore = GImGui->InputEventsQueue.Size;
        io.AddMouseButtonEvent(button, down);
        MarkReplayed(sizeBefore);
    }
    void AddReplayedPosEvent(ImGuiIO& io, ImVec2 pos)
    {
        int sizeBefore = GImGui->InputEventsQueue.Size;
        io.AddMousePosEvent(pos.x, pos.y);
        MarkReplayed(sizeBefore);
    }

    // The right click of a long press, where the finger was. The events carry the mouse source: with the touch source,
    // ImGui's trickling would deliver the position and each button event in separate frames.
    void RightClick(ImGuiIO& io, ImVec2 pos)
    {
        ImGuiMouseSource source = io.MouseSource;
        io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
        AddReplayedPosEvent(io, pos);
        AddReplayedButtonEvent(io, true, ImGuiMouseButton_Right);
        AddReplayedButtonEvent(io, false, ImGuiMouseButton_Right);
        io.AddMouseSourceEvent(source);
    }

    // The press goes to the widget under the finger: a release then a press, through the queue (two frames, with the
    // trickling rules), which the layer must not claim again. The finger's position comes first: a backend may have
    // queued an invalid one already (a finger lifted), and the replayed press must land where the finger was.
    void ReplayPress(ImGuiIO& io, State& s, bool fingerDown)
    {
        // ImGui counted the finger's press as a click when it happened: the replayed press would be the second
        // click of a double click (a tap on a word of a text input selected the word). It pairs with the previous
        // replayed press instead, so that two taps are a double click, as two clicks are. The rule is the layer's,
        // with a finger's time and distance (two taps land farther apart than two clicks); ImGui's own check at the
        // replayed press is made to agree: the previous click is set at the finger, now, or long ago.
        ImGuiContext& g = *GImGui;
        const ImVec2 pos = io.MousePos;
        const bool touch = (io.MouseSource == ImGuiMouseSource_TouchScreen);
        const float maxDist = touch ? g.FontSize * kDoubleTapFontSizes : io.MouseDoubleClickMaxDist;
        const double maxTime = touch ? kDoubleTapSeconds : io.MouseDoubleClickTime;
        bool repeated = (g.Time - s.lastReplayTime) < maxTime && ImLengthSqr(pos - s.lastReplayPos) < maxDist * maxDist;
        io.MouseClickedTime[ImGuiMouseButton_Left] = repeated ? g.Time : -1e9;
        io.MouseClickedPos[ImGuiMouseButton_Left] = pos;
        io.MouseClickedLastCount[ImGuiMouseButton_Left] = (ImU16)s.lastReplayCount;
        s.lastReplayTime = g.Time;
        s.lastReplayPos = pos;
        s.lastReplayCount = repeated ? s.lastReplayCount + 1 : 1;
        io.AddMousePosEvent(pos.x, pos.y);
        if (fingerDown)
            AddReplayedButtonEvent(io, false);
        AddReplayedButtonEvent(io, true);
        if (!fingerDown)
            AddReplayedButtonEvent(io, false);
        s.replayedPresses++;
    }
}  // namespace

void UpdateTouchScroll(TouchScrollMode mode, bool longPressIsRightClick, bool thinScrollbars)
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    State& s = gState;
    if (s.context != &g || g.FrameCount < s.frameCount)
    {
        s = State{};
        s.context = &g;
    }
    s.frameCount = g.FrameCount;
    const ImGuiID id = SentinelId();
    s.previousTakers.swap(s.takers);
    s.takers.clear();

    // Thin bars on a touch screen: indicators, not controls. At each frame, before any window: a theme applied later,
    // or a pinch (the font size), would change them
    if (thinScrollbars && (io.ConfigFlags & ImGuiConfigFlags_IsTouchScreen))
    {
        g.Style.ScrollbarSize = ImMax(1.f, ImFloor(g.FontSize * kThinScrollbarFontSizes));
        g.Style.ScrollbarRounding = g.Style.ScrollbarSize;  // round ends
    }

    bool enabled = (mode == TouchScrollMode::Always)
                   || (mode == TouchScrollMode::Auto && io.MouseSource == ImGuiMouseSource_TouchScreen);
    if (!enabled)
    {
        if (s.owning && g.ActiveId == id)
            ImGui::ClearActiveID();
        EndPress(s);
        s.inertia = ImVec2(0.f, 0.f);
        s.overscroll = s.overscrollSpeed = 0.f;
        return;
    }
    const float dt = (io.DeltaTime > 0.f) ? io.DeltaTime : 1.f / 60.f;

    // A press: one replayed by the layer goes to the widgets; a real one stops the inertia and is claimed when it
    // lands on the content of a window, or on its scroll bars with a finger (not its title bar, nor its borders)
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        if (s.replayedPresses > 0)
            s.replayedPresses--;
        else
        {
            s.inertia = ImVec2(0.f, 0.f);
            s.overscroll = s.overscrollSpeed = 0.f;  // a bounce ends at once: the widgets are where they are drawn
            s.pressTime = (float)g.Time;
            s.pressPos = io.MousePos;
            ImGuiWindow* w = g.HoveredWindow;
            const State::DragTaker* taker =
                (w != nullptr && ImGui::IsMousePosValid()) ? FindDragTaker(s, w, io.MousePos) : nullptr;
            s.watchingLongPress = longPressIsRightClick && ImGui::IsMousePosValid()
                                  && (taker == nullptr || taker->longPressIsRightClick);
            bool touch = (io.MouseSource == ImGuiMouseSource_TouchScreen);
            if (w != nullptr && !w->Collapsed && ImGui::IsMousePosValid()
                && (w->InnerRect.Contains(io.MousePos) || touch && OnScrollbar(w, io.MousePos))
                && CanScrollSomewhere(w) && taker == nullptr)
            {
                // A widget active from before (a text input being edited) loses the id, as with a press elsewhere
                ImGui::SetActiveID(id, w);
                ImGui::FocusWindow(w);  // what the press would have done
                io.MouseClickedCount[ImGuiMouseButton_Left] = 1;  // a double click, if any, is the replayed press's
                s.owning = true;
                s.window = w;
                s.pressPos = io.MousePos;
                s.nbSamples = s.nextSample = 0;
                AddSample(s, (float)g.Time, io.MousePos);
            }
        }
    }

    if (s.owning && g.ActiveId != id)  // a widget took the press itself
        EndPress(s);

    if (s.owning && !io.MouseDown[ImGuiMouseButton_Left])  // the release: a flick keeps scrolling, a tap is replayed
    {
        ImGui::ClearActiveID();
        ImVec2 speed = LiftSpeed(s, (float)g.Time, io.MousePos);
        const float minSpeed = kInertiaMinSpeedEm * g.FontSize;
        bool flick = s.swiping && s.pull == 0.f && ImLengthSqr(speed) > minSpeed * minSpeed;  // not from past the end
        if (!s.swiping && !s.parked)
            ReplayPress(io, s, false);
        s.inertia = flick ? speed : ImVec2(0.f, 0.f);
        EndPress(s);
        s.watchingLongPress = s.longPressArmed = false;
    }

    // The long press: a finger still since its press (the layer let the widget have it at the hold), for half a
    // second, arms a right click, fired when it lifts. The release of a replayed handover is not a lift
    // (replayedPresses tells).
    if (s.watchingLongPress)
    {
        bool lifted = !io.MouseDown[ImGuiMouseButton_Left] && s.replayedPresses == 0;
        float slop = Slop(g);
        bool moved = ImGui::IsMousePosValid() && ImLengthSqr(io.MousePos - s.pressPos) > slop * slop;
        // A widget that acted on the press already (a button that repeats while held, a press-on-click button) keeps
        // it: its action was the press
        bool widgetActed = (g.ActiveId != 0 && g.ActiveId != id && g.ActiveIdHasBeenPressedBefore);
        if (lifted && s.longPressArmed)
        {
            // The widgets see the release at no position, outside them: a button does not click, a text input stays
            // active (the release came this frame, before them)
            io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
            RightClick(io, s.pressPos);
            s.watchingLongPress = s.longPressArmed = false;
        }
        else if (lifted || moved || s.owning && s.swiping || s.parked || widgetActed)
            s.watchingLongPress = s.longPressArmed = false;  // the widget under the finger keeps its press
        else if (!s.owning && (float)g.Time - s.pressTime >= kLongPressSeconds)
            s.longPressArmed = true;
    }

    // A finger that lifted (ours or a widget's): no pointer until the next touch. Queued after a replayed tap, whose
    // press and release need the position (the queue keeps the order). Not on the release of a handover (the finger
    // is still down: the widget would see its drag at no position, a selection jumped to the start of its text).
    if (io.MouseSource == ImGuiMouseSource_TouchScreen && ImGui::IsMouseReleased(ImGuiMouseButton_Left)
        && s.replayedPresses == 0)
        io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);

    if (s.owning && s.parked)
        ImGui::KeepAliveID(id);
    else if (s.owning)
    {
        ImGui::KeepAliveID(id);
        AddSample(s, (float)g.Time, io.MousePos);
        ImVec2 delta = io.MouseDelta;
        if (!s.swiping)
        {
            ImVec2 fromPress = io.MousePos - s.pressPos;
            float slop = Slop(g);
            if (ImLengthSqr(fromPress) > slop * slop)
            {
                s.axis = SwipeAxis(fromPress);
                ImGuiWindow* target = ScrollTarget(s.window, s.axis);
                bool canScroll = (s.axis == ImGuiAxis_X) ? (target->ScrollMax.x != 0.f) : (target->ScrollMax.y != 0.f);
                if (canScroll)
                {
                    s.swiping = true;
                    s.window = target;
                    delta = fromPress;  // the content catches up with the finger
                }
                else
                {
                    // Nothing scrolls that way (a horizontal drag on a page that scrolls vertically): the widget
                    // under the finger gets the press, and the drag from now on, without the hold
                    ImGui::ClearActiveID();
                    ReplayPress(io, s, true);
                    EndPress(s);
                }
            }
            else if (io.MouseDownDuration[ImGuiMouseButton_Left] >= kHoldSeconds)
            {
                // The hold: the widget under the finger gets the press, and the finger's drag from now on
                ImGui::ClearActiveID();
                ReplayPress(io, s, true);
                EndPress(s);
                s.rippleTime = g.Time;
                s.ripplePos = io.MousePos;
            }
        }
        if (s.swiping)
        {
            float motion = Along(delta, s.axis);
            if (s.pull * motion < 0.f)  // the finger comes back: the rubber band gives way before the content scrolls
            {
                float left = s.pull + motion;
                bool stillPulled = (left * s.pull > 0.f);
                s.pull = stillPulled ? left : 0.f;
                motion = stillPulled ? 0.f : left;
            }
            if (motion != 0.f)
                s.pull += ScrollBy(s.window, s.axis, motion);
            s.overscroll = RubberBand(s.pull, s.window, s.axis);
            s.overscrollSpeed = 0.f;
        }
    }

    if (s.inertia.x != 0.f || s.inertia.y != 0.f)
    {
        float speed = Along(s.inertia, s.axis);
        float past = ScrollBy(s.window, s.axis, speed * dt);
        s.inertia = s.inertia * std::exp(-kInertiaDecay * dt);
        const float minSpeed = kInertiaMinSpeedEm * g.FontSize;
        if (past != 0.f)  // the end: the content overshoots at the speed it had, then springs back
        {
            s.overscroll = past;
            s.overscrollSpeed = speed;
            s.inertia = ImVec2(0.f, 0.f);
        }
        else if (ImLengthSqr(s.inertia) < minSpeed * minSpeed)
            s.inertia = ImVec2(0.f, 0.f);
    }

    DrawHoldRipple(s);
    DrawLongPressRing(s);

    // The content past its end springs back, unless a finger holds it there. The step is bounded: a long frame
    // would make the integration overshoot
    if (s.pull == 0.f && (s.overscroll != 0.f || s.overscrollSpeed != 0.f))
    {
        float step = ImMin(dt, 1.f / 30.f);
        s.overscrollSpeed += (-2.f * kBounceOmega * s.overscrollSpeed - kBounceOmega * kBounceOmega * s.overscroll) * step;
        s.overscroll += s.overscrollSpeed * step;
        if (ImFabs(s.overscroll) < 0.5f && ImFabs(s.overscrollSpeed) < kBounceOmega)
            s.overscroll = s.overscrollSpeed = 0.f;
    }

    // The inertia, the bounce and the ripple move without input events: the app does not idle meanwhile
    bool rippling = g.Time - s.rippleTime < kHoldRippleSeconds;
    if (s.inertia.x != 0.f || s.inertia.y != 0.f || s.overscroll != 0.f || s.overscrollSpeed != 0.f || rippling)
        RequestRefresh();
}

namespace
{
    // Moves the draw commands of a window by the overscroll, their clip rects with them, inside the clip rect of the
    // window that scrolls. From the first vertex of the first command in `cmds`: a window's decorations (its
    // background, border and scrollbars, drawn first) stay in place, a child window moves whole.
    void TranslateDrawList(ImDrawList* dl, int firstCmd, ImVec2 offset, const ImRect& clip)
    {
        if (firstCmd >= dl->CmdBuffer.Size)
            return;
        int firstVtx = dl->VtxBuffer.Size;
        for (int i = firstCmd; i < dl->CmdBuffer.Size; ++i)
        {
            ImDrawCmd& cmd = dl->CmdBuffer[i];
            if (cmd.ElemCount == 0 || cmd.UserCallback != nullptr)
                continue;
            firstVtx = ImMin(firstVtx, (int)(dl->IdxBuffer[cmd.IdxOffset] + cmd.VtxOffset));
            ImRect r(cmd.ClipRect.x + offset.x, cmd.ClipRect.y + offset.y, cmd.ClipRect.z + offset.x, cmd.ClipRect.w + offset.y);
            r.ClipWithFull(clip);
            cmd.ClipRect = ImVec4(r.Min.x, r.Min.y, r.Max.x, r.Max.y);
        }
        for (int i = firstVtx; i < dl->VtxBuffer.Size; ++i)
            dl->VtxBuffer[i].pos += offset;
    }

    void TranslateChildren(ImGuiWindow* w, ImVec2 offset, const ImRect& clip)
    {
        for (ImGuiWindow* child : w->DC.ChildWindows)
        {
            if (!child->Active || (child->Flags & ImGuiWindowFlags_Popup))
                continue;
            TranslateDrawList(child->DrawList, 0, offset, clip);
            TranslateChildren(child, offset, clip);
        }
    }
}  // namespace

void ApplyTouchOverscroll()
{
    State& s = gState;
    ImGuiWindow* w = s.window;
    if (s.overscroll == 0.f || w == nullptr || s.context != GImGui || !w->Active)
        return;
    ImVec2 offset = (s.axis == ImGuiAxis_X) ? ImVec2(s.overscroll, 0.f) : ImVec2(0.f, s.overscroll);
    const ImRect clip = w->InnerClipRect;
    // The window's decorations are clipped to its outer rect, its content to the inner rect: the content starts at
    // the first command clipped inside it
    ImDrawList* dl = w->DrawList;
    int firstCmd = dl->CmdBuffer.Size;
    for (int i = 0; i < dl->CmdBuffer.Size; ++i)
    {
        const ImVec4& c = dl->CmdBuffer[i].ClipRect;
        if (dl->CmdBuffer[i].ElemCount > 0 && clip.Contains(ImRect(c.x, c.y, c.z, c.w)))
        {
            firstCmd = i;
            break;
        }
    }
    TranslateDrawList(dl, firstCmd, offset, clip);
    TranslateChildren(w, offset, clip);
}

bool TouchScrollLetGo(bool evenAWidget)
{
    ImGuiContext& g = *GImGui;
    State& s = gState;
    const ImGuiID id = SentinelId();
    s.inertia = ImVec2(0.f, 0.f);
    s.pull = 0.f;
    if (s.owning)
    {
        s.swiping = false;
        s.parked = true;
        return true;
    }
    if (g.ActiveId == 0 || g.ActiveIdWindow == nullptr)
        return true;
    if (!evenAWidget)
        return false;
    ImGui::SetActiveID(id, g.ActiveIdWindow);  // over the widget's id: it sees it lost the press
    s.owning = true;
    s.parked = true;
    return true;
}

bool TouchScrollRelease(bool evenAWidget)
{
    ImGuiContext& g = *GImGui;
    State& s = gState;
    const ImGuiID id = SentinelId();
    s.inertia = ImVec2(0.f, 0.f);
    s.watchingLongPress = s.longPressArmed = false;
    if (s.owning)
    {
        if (g.ActiveId == id)
            ImGui::ClearActiveID();
        EndPress(s);
        return true;
    }
    if (g.ActiveId == 0)
        return true;
    if (!evenAWidget)
        return false;
    ImGui::ClearActiveID();
    return true;
}

// The widget just drawn: its visible part, noted for the press of the next frame. Nothing when the layer did not run
// this frame (an app with its own loop, without HelloImGui's runner): the notes would pile up, never read.
void SetItemTakesTouchDrags(bool longPressIsRightClick)
{
    ImGuiContext& g = *GImGui;
    if (gState.context != &g || gState.frameCount != g.FrameCount)
        return;
    ImGuiWindow* window = g.CurrentWindow;
    ImRect rect = g.LastItemData.Rect;
    rect.ClipWith(window->ClipRect);
    if (rect.GetWidth() > 0.f && rect.GetHeight() > 0.f)
        gState.takers.push_back({window->ID, rect, longPressIsRightClick});
}

}  // namespace HelloImGui
