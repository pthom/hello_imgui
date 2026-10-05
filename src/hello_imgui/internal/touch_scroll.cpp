#define IMGUI_DEFINE_MATH_OPERATORS  // before any include of imgui.h (runner_params.h includes it)
#include "hello_imgui/internal/touch_scroll.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <cmath>

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
// A widget that takes the active id itself ends the swipe (the layer steps aside).
// On a touch screen, there is no pointer between two touches: when a finger lifts, the mouse position becomes
// invalid (as when a mouse leaves the window), so that nothing is hovered while the content coasts, nor after a tap.
// A finger still for half a second is a long press: a right click (the context menus), as on a phone. The widget
// under it holds the left press by then (the hold): it is taken away first, by a release at an invalid position.
namespace HelloImGui
{
namespace
{
    // Tunables. The slop is in font sizes, so that it follows the DPI and the font scale.
    constexpr float kSlopFontSizes = 0.5f;      // a press that moved less than this is a tap, not a swipe
    constexpr float kHoldSeconds = 0.15f;       // a finger still for this long hands the press to the widget under it (iOS: 150 ms)
    constexpr float kLongPressSeconds = 0.5f;   // a finger still for this long is a right click (iOS: about 500 ms)
    constexpr float kInertiaDecay = 2.f;        // speed *= exp(-decay * dt) after the release (iOS: 0.998 per ms)
    constexpr float kInertiaMinSpeedEm = 3.f;   // font sizes per second: below this, a release starts no inertia, and the inertia ends
    constexpr float kFlickWindow = 0.05f;       // s: the lift speed is the finger's motion over this long before the lift
    constexpr int kFlickSamples = 16;           // enough for the window at 240 fps

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
        int replayedPresses = 0;        // presses queued by the layer, which it must not claim
        bool watchingLongPress = false; // a touch press, still so far: a long press when it stays
        float pressTime = 0.f;
    };
    State gState;

    ImGuiID SentinelId() { return ImHashStr("##HelloImGui_TouchScroll"); }

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

    // Scrolls the window by the finger's motion (the content follows the finger). Applied by its next Begin(),
    // which is later in this frame. Returns false when the window is at its end in that direction.
    bool ScrollBy(ImGuiWindow* w, ImGuiAxis axis, float motion)
    {
        if (axis == ImGuiAxis_X)
        {
            ImGui::SetScrollX(w, w->Scroll.x - motion);
            return (motion > 0.f) ? (w->Scroll.x > 0.f) : (w->Scroll.x < w->ScrollMax.x);
        }
        ImGui::SetScrollY(w, w->Scroll.y - motion);
        return (motion > 0.f) ? (w->Scroll.y > 0.f) : (w->Scroll.y < w->ScrollMax.y);
    }

    float Along(ImVec2 v, ImGuiAxis axis) { return axis == ImGuiAxis_X ? v.x : v.y; }

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

    // The long press: the left press, held by the widget under the finger, is taken away by a release at an
    // invalid position (not a click: the release is outside), then the right button clicks where the finger is.
    // The events carry the mouse source: with the touch source, ImGui's trickling delivers a position and the
    // button event that follows it in separate frames, and the widget would see a frame with the button down at no
    // position (a selection jumped to the start of its text, a slider to its minimum).
    void RightClick(ImGuiIO& io, ImVec2 pos)
    {
        ImGuiMouseSource source = io.MouseSource;
        io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
        AddReplayedPosEvent(io, ImVec2(-FLT_MAX, -FLT_MAX));
        AddReplayedButtonEvent(io, false, ImGuiMouseButton_Left);
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
        // click of a double click (a tap on a word of a text input selected the word)
        io.MouseClickedTime[ImGuiMouseButton_Left] = -1e9;
        io.AddMousePosEvent(io.MousePos.x, io.MousePos.y);
        if (fingerDown)
            AddReplayedButtonEvent(io, false);
        AddReplayedButtonEvent(io, true);
        if (!fingerDown)
            AddReplayedButtonEvent(io, false);
        s.replayedPresses++;
    }
}  // namespace

void UpdateTouchScroll(TouchScrollMode mode, bool longPressIsRightClick)
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

    bool enabled = (mode == TouchScrollMode::Always)
                   || (mode == TouchScrollMode::Auto && io.MouseSource == ImGuiMouseSource_TouchScreen);
    if (!enabled)
    {
        if (s.owning && g.ActiveId == id)
            ImGui::ClearActiveID();
        EndPress(s);
        s.inertia = ImVec2(0.f, 0.f);
        return;
    }
    const float dt = (io.DeltaTime > 0.f) ? io.DeltaTime : 1.f / 60.f;

    // A press: one replayed by the layer goes to the widgets; a real one stops the inertia and is claimed when it
    // lands on the content of a window (not its title bar, its scrollbars, its borders)
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        if (s.replayedPresses > 0)
            s.replayedPresses--;
        else
        {
            s.inertia = ImVec2(0.f, 0.f);
            s.watchingLongPress = longPressIsRightClick && ImGui::IsMousePosValid();
            s.pressTime = (float)g.Time;
            s.pressPos = io.MousePos;
            ImGuiWindow* w = g.HoveredWindow;
            if (w != nullptr && !w->Collapsed && ImGui::IsMousePosValid() && w->InnerRect.Contains(io.MousePos)
                && CanScrollSomewhere(w))
            {
                // A widget active from before (a text input being edited) loses the id, as with a press elsewhere
                ImGui::SetActiveID(id, w);
                ImGui::FocusWindow(w);  // what the press would have done
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
        bool flick = s.swiping && ImLengthSqr(speed) > minSpeed * minSpeed;
        if (!s.swiping && !s.parked)
            ReplayPress(io, s, false);
        s.inertia = flick ? speed : ImVec2(0.f, 0.f);
        EndPress(s);
        s.watchingLongPress = false;
    }

    // The long press: a finger still since its press (the layer let the widget have it at the hold), for half a
    // second. The release of a replayed handover is not a lift (replayedPresses tells).
    if (s.watchingLongPress)
    {
        bool lifted = !io.MouseDown[ImGuiMouseButton_Left] && s.replayedPresses == 0;
        float slop = g.FontSize * kSlopFontSizes;
        bool moved = ImGui::IsMousePosValid() && ImLengthSqr(io.MousePos - s.pressPos) > slop * slop;
        if (lifted || moved || s.owning && s.swiping || s.parked)
            s.watchingLongPress = false;
        else if (!s.owning && (float)g.Time - s.pressTime >= kLongPressSeconds)
        {
            s.watchingLongPress = false;
            RightClick(io, s.pressPos);
        }
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
            float slop = g.FontSize * kSlopFontSizes;
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
            }
        }
        if (s.swiping)
        {
            float motion = Along(delta, s.axis);
            if (motion != 0.f)
                ScrollBy(s.window, s.axis, motion);
        }
    }

    if (s.inertia.x != 0.f || s.inertia.y != 0.f)
    {
        bool moving = ScrollBy(s.window, s.axis, Along(s.inertia, s.axis) * dt);
        s.inertia = s.inertia * std::exp(-kInertiaDecay * dt);
        const float minSpeed = kInertiaMinSpeedEm * g.FontSize;
        if (!moving || ImLengthSqr(s.inertia) < minSpeed * minSpeed)
            s.inertia = ImVec2(0.f, 0.f);
    }
}

bool TouchScrollLetGo(bool evenAWidget)
{
    ImGuiContext& g = *GImGui;
    State& s = gState;
    const ImGuiID id = SentinelId();
    s.inertia = ImVec2(0.f, 0.f);
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

}  // namespace HelloImGui
