// imgui_kit: the swipe of a touch screen (ImGuiTouch)
// https://github.com/pthom/imgui_kit (MIT license)

/*

Index of this file:
// [SECTION] Tunables and helpers
// [SECTION] The phases of a frame
// [SECTION] UpdateTouchScroll
// [SECTION] The overscroll (ApplyTouchOverscroll)
// [SECTION] Let go, release (the pinch)
// [SECTION] The scroll bar held by a finger
// [SECTION] The widgets that take the drags

*/

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_kit.h"
#include "imgui_kit_touch_internal.h"

#include "imgui.h"
#include "imgui_internal.h"
#include <math.h>  // expf

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
// SetItemTakesDrags() at the last frame gets a press on it at once: on a touch screen nothing is hovered before
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
namespace ImGuiTouch
{
//-----------------------------------------------------------------------------
// [SECTION] Tunables and helpers
//-----------------------------------------------------------------------------

// Tunables. The slop is in font sizes, so that it follows the DPI and the font scale.
// A press that moved less than the slop is a tap, not a swipe: a mouse's, and a finger's (it jitters more, and is
// a finger wide)
constexpr float SLOP_FONT_SIZES = 0.5f;
constexpr float SLOP_FONT_SIZES_TOUCH = 1.0f;
constexpr float HOLD_RIPPLE_SECONDS = 0.35f;  // the ring drawn around the finger when the hold hands it the press
// Two taps closer than this in time and in distance are a double click (ImGui's 0.3 s and 6 px suit a mouse)
constexpr float DOUBLE_TAP_SECONDS = 0.4f;
constexpr float DOUBLE_TAP_FONT_SIZES = 1.5f;
constexpr float LONG_PRESS_ARC_DELAY = 0.15f;  // s: the long press's arc shows after this, so that a tap does not
                                             // flash it
constexpr float LONG_PRESS_RING_RADIUS = 3.0f;  // font sizes: the long press's ring, past a fingertip
constexpr float INERTIA_DECAY = 2.0f;         // speed *= exp(-decay * dt) after the release (iOS: 0.998 per ms)
constexpr float INERTIA_MIN_SPEED_EM = 3.0f;    // font sizes per second: below this, a release starts no inertia, and
                                             // the inertia ends
constexpr float FLICK_WINDOW = 0.05f;        // s: the lift speed is the finger's motion over this long before the
                                             // lift
// iOS's rubber band: a drag of d past the end moves the content by (1 - 1 / (d * c / size + 1)) * size
constexpr float RUBBER_BAND_RESISTANCE = 0.55f;
constexpr float THIN_SCROLLBAR_FONT_SIZES = 0.3f;  // the scroll bars' width on a touch screen (iOS: about 3 pt)
constexpr float HELD_SCROLLBAR_FONT_SIZES = 0.8f;  // a bar held by a finger widens to this, over the content (iOS
                                                 // too)
constexpr float HELD_SCROLLBAR_GROW_SECONDS = 0.15f;
constexpr float BOUNCE_OMEGA = 12.0f;  // 1/s: the content past its end springs back as a critically damped spring
                                      // (settled in about 0.4 s)

const ImGuiID SENTINEL_ID = ImHashStr("##ImGuiTouch_Scroll");  // the active id the layer holds during a press
static ImGuiID SentinelId() { return SENTINEL_ID; }

static float Slop(const ImGuiContext& g)
{
    return g.FontSize * (g.IO.MouseSource == ImGuiMouseSource_TouchScreen ? SLOP_FONT_SIZES_TOUCH : SLOP_FONT_SIZES);
}

// The feedback of the hold: a ring that grows and fades around the finger, so that the user knows the widget
// under it has the press, and the drag can start
// The viewport under the pointer (a window on a second monitor, with viewports enabled), else the main one
static ImDrawList* ForegroundDrawList()
{
    ImGuiContext& g = *GImGui;
    return ImGui::GetForegroundDrawList(g.HoveredWindow ? g.HoveredWindow->Viewport : ImGui::GetMainViewport());
}

static void DrawHoldRipple(const ImGuiTouchScrollState& s)
{
    ImGuiContext& g = *GImGui;
    float t = (float)(g.Time - s.RippleTime) / HOLD_RIPPLE_SECONDS;
    if (t < 0.0f || t > 1.0f || !ImGui::IsMousePosValid(&s.RipplePos))
        return;
    float radius = g.FontSize * (2.0f + 1.5f * t);  // from past a fingertip
    ImU32 col = ImGui::GetColorU32(ImGuiCol_Text, 0.9f * (1.0f - t));
    ForegroundDrawList()->AddCircle(s.RipplePos, radius, col, 0, g.FontSize * 0.25f);
}

// The long press: while the finger stays still, an arc fills around it (from the top, clockwise); once armed, a
// steady ring, until the finger lifts (a right click) or moves (cancelled). Its radius goes past a fingertip.
static void DrawLongPressRing(const ImGuiTouchScrollState& s)
{
    ImGuiContext& g = *GImGui;
    if (!s.WatchingLongPress || !ImGui::IsMousePosValid())
        return;
    ImDrawList* draw_list = ForegroundDrawList();
    float radius = g.FontSize * LONG_PRESS_RING_RADIUS;
    float thickness = g.FontSize * 0.3f;
    if (s.LongPressArmed)
    {
        draw_list->AddCircle(g.IO.MousePos, radius, ImGui::GetColorU32(ImGuiCol_Text, 0.3f), 0, thickness);
        return;
    }
    // The arc shows after a short delay: a tap does not flash it
    const float long_press = ImMax(GetTouchContext().Config.LongPressSeconds, LONG_PRESS_ARC_DELAY + 0.01f);
    float t = ((float)g.Time - s.PressTime - LONG_PRESS_ARC_DELAY) / (long_press - LONG_PRESS_ARC_DELAY);
    if (t <= 0.0f)
        return;
    draw_list->AddCircle(g.IO.MousePos, radius, ImGui::GetColorU32(ImGuiCol_Text, 0.08f), 0, thickness);
    float a0 = -IM_PI * 0.5f;
    draw_list->PathArcTo(g.IO.MousePos, radius, a0, a0 + 2.0f * IM_PI * ImMin(t, 1.0f));
    draw_list->PathStroke(ImGui::GetColorU32(ImGuiCol_Text, 0.3f), thickness);
}

// The axis of a swipe: the dominant direction of the finger (a swipe scrolls one axis, like the wheel)
static ImGuiAxis SwipeAxis(ImVec2 from_press)
{
    return (ImFabs(from_press.x) > ImFabs(from_press.y)) ? ImGuiAxis_X : ImGuiAxis_Y;
}

// Whether a window's flags let a pointer scroll it (Dear ImGui's rule, in FindBestWheelingWindow):
// NoScrollWithMouse refuses, unless NoMouseInputs is set too (the window then passes everything to its parent)
static bool ScrollsWithPointer(const ImGuiWindow* w)
{
    return !(w->Flags & ImGuiWindowFlags_NoScrollWithMouse) || (w->Flags & ImGuiWindowFlags_NoMouseInputs);
}

// The window to scroll on this axis, as ImGui's wheel chooses it (FindBestWheelingWindow): the pressed one, or
// the first parent that can scroll that way. A child at the end of its scroll keeps the swipe, like the wheel.
static ImGuiWindow* ScrollTarget(ImGuiWindow* w, ImGuiAxis axis)
{
    for (; w->Flags & ImGuiWindowFlags_ChildWindow; w = w->ParentWindow)
    {
        bool can_scroll = (axis == ImGuiAxis_X) ? (w->ScrollMax.x != 0.0f) : (w->ScrollMax.y != 0.0f);
        if (can_scroll && ScrollsWithPointer(w))
            break;
    }
    return w;
}

// The widget under the press that asked for the drags at the last frame (SetItemTakesDrags), if any: in the
// pressed window, or in a window that holds it as a child (a plot drawn in a child window of its own)
const ImGuiTouchScrollState::DragTaker* FindDragTaker(const ImGuiTouchScrollState& s, const ImGuiWindow* w, ImVec2 pos)
{
    for (const ImGuiTouchScrollState::DragTaker& t : s.PreviousTakers)
    {
        if (!t.Rect.Contains(pos))
            continue;
        for (const ImGuiWindow* x = w; x; x = (x->Flags & ImGuiWindowFlags_ChildWindow) ? x->ParentWindow : nullptr)
            if (x->ID == t.Window)
                return &t;
    }
    return nullptr;
}

// With a finger, a window's scroll bars are indicators: a press on one is a swipe, as on the content (dragging
// the thumb moved the content against the finger, and faster than it). A hold still hands it the thumb.
static bool OnScrollbar(ImGuiWindow* w, ImVec2 pos)
{
    return (w->ScrollbarY && ImGui::GetWindowScrollbarRect(w, ImGuiAxis_Y).Contains(pos))
        || (w->ScrollbarX && ImGui::GetWindowScrollbarRect(w, ImGuiAxis_X).Contains(pos));
}

// Whether the window, or a parent, can scroll at all: when nothing can, a press has nothing to pre-empt
static bool CanScrollSomewhere(const ImGuiWindow* w)
{
    for (;; w = w->ParentWindow)
    {
        bool can_scroll = (w->ScrollMax.x != 0.0f || w->ScrollMax.y != 0.0f) && ScrollsWithPointer(w);
        if (can_scroll)
            return true;
        if (!(w->Flags & ImGuiWindowFlags_ChildWindow))
            return false;
    }
}

// Scrolls the window by the finger's motion (the content follows the finger). Applied by its next Begin(),
// which is later in this frame. Returns the part of the motion past the end of the scroll (zero when all of it
// scrolls).
static float ScrollBy(ImGuiWindow* w, ImGuiAxis axis, float motion)
{
    float scroll = w->Scroll[axis], scroll_max = w->ScrollMax[axis];
    float room = (motion > 0.0f) ? scroll : scroll_max - scroll;  // how far the content can still go that way
    float absorbed = ImMin(ImFabs(motion), room);
    if (axis == ImGuiAxis_X)
        ImGui::SetScrollX(w, scroll - motion);
    else
        ImGui::SetScrollY(w, scroll - motion);
    return motion - ((motion > 0.0f) ? absorbed : -absorbed);
}

// The rubber band of iOS: the content follows a finger past the end, by less and less, never past the window
static float RubberBand(float pull, const ImGuiWindow* w, ImGuiAxis axis)
{
    float size = (axis == ImGuiAxis_X) ? w->InnerRect.GetWidth() : w->InnerRect.GetHeight();
    if (pull == 0.0f || size <= 0.0f)
        return 0.0f;
    float d = (1.0f - 1.0f / (ImFabs(pull) * RUBBER_BAND_RESISTANCE / size + 1.0f)) * size;
    return (pull > 0.0f) ? d : -d;
}

static void AddSample(ImGuiTouchScrollState& s, float time, ImVec2 pos)
{
    s.Samples[s.NextSample] = {time, pos};
    s.NextSample = (s.NextSample + 1) % IMGUI_TOUCH_FLICK_SAMPLES;
    s.NbSamples = ImMin(s.NbSamples + 1, IMGUI_TOUCH_FLICK_SAMPLES);
}

// The speed of the finger at the lift: its motion over the last k_flick_window seconds (a quick flick accelerates
// until the finger leaves: an average over a longer time would lag behind it). A finger that paused before the
// lift gives zero.
static ImVec2 LiftSpeed(const ImGuiTouchScrollState& s, float now, ImVec2 pos)
{
    if (!ImGui::IsMousePosValid(&pos))  // a backend that lost the pointer before the release: no fling from it
        return ImVec2(0.0f, 0.0f);
    const ImGuiTouchScrollSample* oldest = nullptr;
    for (int i = 0; i < s.NbSamples; ++i)
    {
        const ImGuiTouchScrollSample& sample = s.Samples[i];
        if (sample.Time <= now - FLICK_WINDOW && (oldest == nullptr || sample.Time > oldest->Time))
            oldest = &sample;  // the most recent sample at or before the window
    }
    if (oldest == nullptr)  // the press is younger than the window: all of it
        for (int i = 0; i < s.NbSamples; ++i)
            if (oldest == nullptr || s.Samples[i].Time < oldest->Time)
                oldest = &s.Samples[i];
    float span = oldest ? now - oldest->Time : 0.0f;
    return (span > 0.0f) ? (pos - oldest->Pos) / span : ImVec2(0.0f, 0.0f);
}

static void EndPress(ImGuiTouchScrollState& s)
{
    s.Owning = false;
    s.Swiping = false;
    s.Parked = false;
    s.Pull = 0.0f;  // a content pulled past its end springs back
}

// A replayed event: AddedByTestEngine means "not from the backend". The Test Engine, when it drives the app, erases
// at each frame the queued events it did not add (the backend's, which would fight its own): the layer's replays
// must survive it. The retagging of the mouse's source (SetupEmscripten, SimulateTouch) skips them too.
static void MarkReplayed(int size_before)
{
    ImGuiContext& g = *GImGui;
    if (g.InputEventsQueue.Size > size_before)
        g.InputEventsQueue.back().AddedByTestEngine = true;
}
static void AddReplayedButtonEvent(ImGuiIO& io, bool down, ImGuiMouseButton button = ImGuiMouseButton_Left)
{
    int size_before = GImGui->InputEventsQueue.Size;
    io.AddMouseButtonEvent(button, down);
    MarkReplayed(size_before);
}
static void AddReplayedPosEvent(ImGuiIO& io, ImVec2 pos)
{
    int size_before = GImGui->InputEventsQueue.Size;
    io.AddMousePosEvent(pos.x, pos.y);
    MarkReplayed(size_before);
}

// The right click of a long press, where the finger was. The events carry the mouse source: with the touch source,
// ImGui's trickling would deliver the position and each button event in separate frames.
static void RightClick(ImGuiIO& io, ImVec2 pos)
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
static void ReplayPress(ImGuiIO& io, ImGuiTouchScrollState& s, bool finger_down)
{
    // ImGui counted the finger's press as a click when it happened: the replayed press would be the second
    // click of a double click (a tap on a word of a text input selected the word). It pairs with the previous
    // replayed press instead, so that two taps are a double click, as two clicks are. The rule is the layer's,
    // with a finger's time and distance (two taps land farther apart than two clicks); ImGui's own check at the
    // replayed press is made to agree: the previous click is set at the finger, now, or long ago.
    ImGuiContext& g = *GImGui;
    // Where the press was, if the backend has no position anymore (in a browser, the focus went to another element
    // just before the release, as the phone keyboard's field: the canvas reported that the pointer left)
    const ImVec2 pos = ImGui::IsMousePosValid(&io.MousePos) ? io.MousePos : s.PressPos;
    const bool touch = (io.MouseSource == ImGuiMouseSource_TouchScreen);
    const float max_dist = touch ? g.FontSize * DOUBLE_TAP_FONT_SIZES : io.MouseDoubleClickMaxDist;
    const double max_time = touch ? DOUBLE_TAP_SECONDS : io.MouseDoubleClickTime;
    bool repeated = (g.Time - s.LastReplayTime) < max_time && ImLengthSqr(pos - s.LastReplayPos) < max_dist * max_dist;
    io.MouseClickedTime[ImGuiMouseButton_Left] = repeated ? g.Time : -1e9;
    io.MouseClickedPos[ImGuiMouseButton_Left] = pos;
    io.MouseClickedLastCount[ImGuiMouseButton_Left] = (ImU16)s.LastReplayCount;
    s.LastReplayTime = g.Time;
    s.LastReplayPos = pos;
    s.LastReplayCount = repeated ? s.LastReplayCount + 1 : 1;
    io.AddMousePosEvent(pos.x, pos.y);
    if (finger_down)
        AddReplayedButtonEvent(io, false);
    AddReplayedButtonEvent(io, true);
    if (!finger_down)
        AddReplayedButtonEvent(io, false);
    s.ReplayedPresses++;
}

//-----------------------------------------------------------------------------
// [SECTION] The phases of a frame
//-----------------------------------------------------------------------------

// Thin bars on a touch screen: indicators, not controls. At each frame, before any window: a theme applied later,
// or a pinch (the font size), would change them. The style's own values are kept, and put back when the bars
// stop being thin; values that differ from the ones written last come from the app (a theme): they are its own.
// The pointer in use decides too: a touch laptop driven by its mouse keeps the theme's bars, until a finger comes.
static void UpdateThinScrollbars(ImGuiTouchScrollState& s, bool thin_scrollbars)
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    if (io.MouseSource == ImGuiMouseSource_TouchScreen)
        s.MouseInUse = false;
    else if (ImGui::IsMousePosValid() && (io.MouseDelta.x != 0.0f || io.MouseDelta.y != 0.0f || io.MouseDown[0]))
        s.MouseInUse = true;
    s.ThinScrollbars = thin_scrollbars && (io.ConfigFlags & ImGuiConfigFlags_IsTouchScreen) && !s.MouseInUse;
    const bool style_changed = s.BarsWritten && (g.Style.ScrollbarSize != s.WrittenBarSize
                                                || g.Style.ScrollbarRounding != s.WrittenBarRounding);
    if (s.ThinScrollbars)
    {
        if (!s.BarsWritten || style_changed)
        {
            s.ThemeBarSize = g.Style.ScrollbarSize;
            s.ThemeBarRounding = g.Style.ScrollbarRounding;
        }
        g.Style.ScrollbarSize = ImMax(1.0f, ImFloor(g.FontSize * THIN_SCROLLBAR_FONT_SIZES));
        g.Style.ScrollbarRounding = g.Style.ScrollbarSize;  // round ends
        s.WrittenBarSize = g.Style.ScrollbarSize;
        s.WrittenBarRounding = g.Style.ScrollbarRounding;
        s.BarsWritten = true;
    }
    else if (s.BarsWritten)
    {
        if (!style_changed)
        {
            g.Style.ScrollbarSize = s.ThemeBarSize;
            g.Style.ScrollbarRounding = s.ThemeBarRounding;
        }
        s.BarsWritten = false;
    }
}

// A press: one replayed by the layer goes to the widgets; a real one stops the inertia and is claimed when it
// lands on the content of a window, or on its scroll bars with a finger (not its title bar, nor its borders)
static void OnPress(ImGuiTouchScrollState& s, ImGuiID id, bool long_press_is_right_click)
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        return;
    if (s.ReplayedPresses > 0)
        s.ReplayedPresses--;
    else
    {
        s.Inertia = ImVec2(0.0f, 0.0f);
        s.Overscroll = s.OverscrollSpeed = 0.0f;  // a bounce ends at once: the widgets are where they are drawn
        s.PressTime = (float)g.Time;
        s.PressPos = io.MousePos;
        ImGuiWindow* w = g.HoveredWindow;
        const ImGuiTouchScrollState::DragTaker* taker =
            (w != nullptr && ImGui::IsMousePosValid()) ? FindDragTaker(s, w, io.MousePos) : nullptr;
        s.WatchingLongPress = long_press_is_right_click && ImGui::IsMousePosValid()
                              && (taker == nullptr || taker->LongPressIsRightClick);
        bool touch = (io.MouseSource == ImGuiMouseSource_TouchScreen);
        // A widget that takes all the drags gets the press at once; one that takes them along an axis gets it
        // once the finger moved that way (below)
        bool takes_all = (taker != nullptr && taker->Axis == ImGuiAxis_None);
        if (w != nullptr && !w->Collapsed && ImGui::IsMousePosValid()
            && (w->InnerRect.Contains(io.MousePos) || (touch && OnScrollbar(w, io.MousePos)))
            && CanScrollSomewhere(w) && !takes_all)
        {
            s.TakerAxis = (taker != nullptr) ? taker->Axis : ImGuiAxis_None;
            // A widget active from before (a text input being edited) loses the id, as with a press elsewhere
            ImGui::SetActiveID(id, w);
            ImGui::FocusWindow(w);  // what the press would have done
            io.MouseClickedCount[ImGuiMouseButton_Left] = 1;  // a double click, if any, is the replayed press's
            s.Owning = true;
            s.Window = w;
            s.PressPos = io.MousePos;
            s.NbSamples = s.NextSample = 0;
            AddSample(s, (float)g.Time, io.MousePos);
        }
    }
}

// The release: a flick keeps scrolling, a tap is replayed
static void OnRelease(ImGuiTouchScrollState& s)
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    if (!s.Owning || io.MouseDown[ImGuiMouseButton_Left])
        return;
    ImGui::ClearActiveID();
    ImVec2 speed = LiftSpeed(s, (float)g.Time, io.MousePos);
    const float min_speed = INERTIA_MIN_SPEED_EM * g.FontSize;
    bool flick = s.Swiping && s.Pull == 0.0f && ImLengthSqr(speed) > min_speed * min_speed;  // not from past the end
    if (!s.Swiping && !s.Parked)
        ReplayPress(io, s, false);
    s.Inertia = flick ? speed : ImVec2(0.0f, 0.0f);
    EndPress(s);
    s.WatchingLongPress = s.LongPressArmed = false;
}

// The long press: a finger still since its press (the layer let the widget have it at the hold), for half a
// second, arms a right click, fired when it lifts. The release of a replayed handover is not a lift
// (ReplayedPresses tells).
static void WatchLongPress(ImGuiTouchScrollState& s, ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    if (!s.WatchingLongPress)
        return;
    bool lifted = !io.MouseDown[ImGuiMouseButton_Left] && s.ReplayedPresses == 0;
    float slop = Slop(g);
    bool moved = ImGui::IsMousePosValid() && ImLengthSqr(io.MousePos - s.PressPos) > slop * slop;
    // A widget that acted on the press already (a button that repeats while held, a press-on-click button) keeps
    // it: its action was the press
    bool widget_acted = (g.ActiveId != 0 && g.ActiveId != id && g.ActiveIdHasBeenPressedBefore);
    if (lifted && s.LongPressArmed)
    {
        // The widgets see the release at no position, outside them: a button does not click, a text input stays
        // active (the release came this frame, before them)
        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
        RightClick(io, s.PressPos);
        s.WatchingLongPress = s.LongPressArmed = false;
    }
    else if (lifted || moved || (s.Owning && s.Swiping) || s.Parked || widget_acted)
        s.WatchingLongPress = s.LongPressArmed = false;  // the widget under the finger keeps its press
    else if (!s.Owning && (float)g.Time - s.PressTime >= GetTouchContext().Config.LongPressSeconds)
        s.LongPressArmed = true;
}

// The finger down, the layer owning its press: the slop and the hold decide between a swipe and a handover; a
// swipe moves the content with the finger, the rubber band past its end
static void FollowFinger(ImGuiTouchScrollState& s, ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    if (!s.Owning)
        return;
    ImGui::KeepAliveID(id);
    if (s.Parked)  // the sentinel holds the press for a pinch: nothing until the fingers lift
        return;
    if (ImGui::IsMousePosValid())  // a pointer lost for a frame leaves no sample: the lift speed stays finite
        AddSample(s, (float)g.Time, io.MousePos);
    ImVec2 delta = io.MouseDelta;
    if (!s.Swiping)
    {
        ImVec2 from_press = io.MousePos - s.PressPos;
        float slop = Slop(g);
        if (ImLengthSqr(from_press) > slop * slop)
        {
            s.Axis = SwipeAxis(from_press);
            ImGuiWindow* target = ScrollTarget(s.Window, s.Axis);
            bool can_scroll =
                (s.Axis == ImGuiAxis_X) ? (target->ScrollMax.x != 0.0f) : (target->ScrollMax.y != 0.0f);
            if (can_scroll && s.Axis != s.TakerAxis)
            {
                s.Swiping = true;
                s.Window = target;
                delta = from_press;  // the content catches up with the finger
            }
            else
            {
                // Nothing scrolls that way (a horizontal drag on a page that scrolls vertically), or the widget
                // under the finger takes the drags along it (a slider): it gets the press, and the drag from now
                // on, without the hold
                ImGui::ClearActiveID();
                ReplayPress(io, s, true);
                EndPress(s);
            }
        }
        else if (io.MouseDownDuration[ImGuiMouseButton_Left] >= GetTouchContext().Config.HoldSeconds)
        {
            // The hold: the widget under the finger gets the press, and the finger's drag from now on
            ImGui::ClearActiveID();
            ReplayPress(io, s, true);
            EndPress(s);
            s.RippleTime = g.Time;
            s.RipplePos = io.MousePos;
        }
    }
    if (s.Swiping)
    {
        float motion = delta[s.Axis];
        if (s.Pull * motion < 0.0f)  // the finger comes back: the rubber band gives way before the content scrolls
        {
            float left = s.Pull + motion;
            bool still_pulled = (left * s.Pull > 0.0f);
            s.Pull = still_pulled ? left : 0.0f;
            motion = still_pulled ? 0.0f : left;
        }
        if (motion != 0.0f)
            s.Pull += ScrollBy(s.Window, s.Axis, motion);
        s.Overscroll = RubberBand(s.Pull, s.Window, s.Axis);
        s.OverscrollSpeed = 0.0f;
    }
}

// After a flick: the content keeps going, slower; at the end it overshoots, then springs back
static void Coast(ImGuiTouchScrollState& s, float dt)
{
    ImGuiContext& g = *GImGui;
    if (!(s.Inertia.x != 0.0f || s.Inertia.y != 0.0f))
        return;
    float speed = s.Inertia[s.Axis];
    float past = ScrollBy(s.Window, s.Axis, speed * dt);
    s.Inertia = s.Inertia * expf(-INERTIA_DECAY * dt);
    const float min_speed = INERTIA_MIN_SPEED_EM * g.FontSize;
    if (past != 0.0f)  // the end: the content overshoots at the speed it had, then springs back
    {
        s.Overscroll = past;
        s.OverscrollSpeed = speed;
        s.Inertia = ImVec2(0.0f, 0.0f);
    }
    else if (ImLengthSqr(s.Inertia) < min_speed * min_speed)
        s.Inertia = ImVec2(0.0f, 0.0f);
}

// The content past its end springs back, unless a finger holds it there. The step is bounded: a long frame
// would make the integration overshoot
static void Bounce(ImGuiTouchScrollState& s, float dt)
{
    if (!(s.Pull == 0.0f && (s.Overscroll != 0.0f || s.OverscrollSpeed != 0.0f)))
        return;
    float step = ImMin(dt, 1.0f / 30.0f);
    s.OverscrollSpeed +=
        (-2.0f * BOUNCE_OMEGA * s.OverscrollSpeed - BOUNCE_OMEGA * BOUNCE_OMEGA * s.Overscroll) * step;
    s.Overscroll += s.OverscrollSpeed * step;
    if (ImFabs(s.Overscroll) < 0.5f && ImFabs(s.OverscrollSpeed) < BOUNCE_OMEGA)
        s.Overscroll = s.OverscrollSpeed = 0.0f;
}

//-----------------------------------------------------------------------------
// [SECTION] UpdateTouchScroll
//-----------------------------------------------------------------------------

void UpdateTouchScroll(ImGuiTouchScrollMode mode, bool long_press_is_right_click, bool thin_scrollbars)
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    ImGuiTouchScrollState& s = GetTouchContext().Scroll;
    if (s.Context != &g || g.FrameCount < s.FrameCount)
    {
        s = ImGuiTouchScrollState{};
        s.Context = &g;
    }
    s.FrameCount = g.FrameCount;
    const ImGuiID id = SentinelId();
    s.PreviousTakers.swap(s.Takers);
    s.Takers.resize(0);
    UpdateThinScrollbars(s, thin_scrollbars);

    bool enabled = (mode == ImGuiTouchScrollMode_Always)
                   || (mode == ImGuiTouchScrollMode_Auto && io.MouseSource == ImGuiMouseSource_TouchScreen);
    if (!enabled)
    {
        if (s.Owning && g.ActiveId == id)
            ImGui::ClearActiveID();
        EndPress(s);
        s.Inertia = ImVec2(0.0f, 0.0f);
        s.Overscroll = s.OverscrollSpeed = 0.0f;
        s.WatchingLongPress = s.LongPressArmed = false;
        return;
    }
    const float dt = (io.DeltaTime > 0.0f) ? io.DeltaTime : 1.0f / 60.0f;

    // The phases, in the order the gesture goes through them in a frame; each one knows whether it applies
    OnPress(s, id, long_press_is_right_click);
    if (s.Owning && g.ActiveId != id)  // a widget took the press itself
        EndPress(s);
    OnRelease(s);
    WatchLongPress(s, id);

    // A finger that lifted (ours or a widget's): no pointer until the next touch. Queued after a replayed tap, whose
    // press and release need the position (the queue keeps the order). Not on the release of a handover (the finger
    // is still down: the widget would see its drag at no position, a selection jumped to the start of its text).
    if (io.MouseSource == ImGuiMouseSource_TouchScreen && ImGui::IsMouseReleased(ImGuiMouseButton_Left)
        && s.ReplayedPresses == 0)
        io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);

    FollowFinger(s, id);
    Coast(s, dt);
    DrawHoldRipple(s);
    DrawLongPressRing(s);
    Bounce(s, dt);

    // The inertia, the bounce and the ripple move without input events: the app does not idle meanwhile
    bool rippling = g.Time - s.RippleTime < HOLD_RIPPLE_SECONDS;
    // The long press's arc fills while the finger stays still, with no input event either
    if (s.Inertia.x != 0.0f || s.Inertia.y != 0.0f || s.Overscroll != 0.0f || s.OverscrollSpeed != 0.0f || rippling
        || (s.WatchingLongPress && !s.LongPressArmed))
        RequestAnimation();
}

//-----------------------------------------------------------------------------
// [SECTION] The overscroll (ApplyTouchOverscroll)
//-----------------------------------------------------------------------------

// Moves the draw commands of a window by the overscroll, their clip rects with them, inside the clip rect of the
// window that scrolls. From the first vertex of the first command in `cmds`: a window's decorations (its
// background, border and scrollbars, drawn first) stay in place, a child window moves whole.
static void TranslateDrawList(ImDrawList* dl, int first_cmd, ImVec2 offset, const ImRect& clip)
{
    if (first_cmd >= dl->CmdBuffer.Size)
        return;
    int first_vtx = dl->VtxBuffer.Size;
    for (int i = first_cmd; i < dl->CmdBuffer.Size; ++i)
    {
        ImDrawCmd& cmd = dl->CmdBuffer[i];
        if (cmd.ElemCount == 0 || cmd.UserCallback != nullptr)
            continue;
        first_vtx = ImMin(first_vtx, (int)(dl->IdxBuffer[cmd.IdxOffset] + cmd.VtxOffset));
        ImRect r(cmd.ClipRect.x + offset.x, cmd.ClipRect.y + offset.y, cmd.ClipRect.z + offset.x,
                 cmd.ClipRect.w + offset.y);
        r.ClipWithFull(clip);
        cmd.ClipRect = ImVec4(r.Min.x, r.Min.y, r.Max.x, r.Max.y);
    }
    for (int i = first_vtx; i < dl->VtxBuffer.Size; ++i)
        dl->VtxBuffer[i].pos += offset;
}

static void TranslateChildren(ImGuiWindow* w, ImVec2 offset, const ImRect& clip)
{
    for (ImGuiWindow* child : w->DC.ChildWindows)
    {
        if (!child->Active || (child->Flags & ImGuiWindowFlags_Popup))
            continue;
        TranslateDrawList(child->DrawList, 0, offset, clip);
        TranslateChildren(child, offset, clip);
    }
}

void ApplyTouchOverscroll()
{
    ImGuiTouchScrollState& s = GetTouchContext().Scroll;
    ImGuiWindow* w = s.Window;
    if (s.Overscroll == 0.0f || w == nullptr || s.Context != GImGui || !w->Active)
        return;
    ImVec2 offset = (s.Axis == ImGuiAxis_X) ? ImVec2(s.Overscroll, 0.0f) : ImVec2(0.0f, s.Overscroll);
    const ImRect clip = w->InnerClipRect;
    // The window's decorations are clipped to its outer rect, its content to the inner rect: the content starts at
    // the first command clipped inside it
    ImDrawList* dl = w->DrawList;
    int first_cmd = dl->CmdBuffer.Size;
    for (int i = 0; i < dl->CmdBuffer.Size; ++i)
    {
        const ImVec4& c = dl->CmdBuffer[i].ClipRect;
        if (dl->CmdBuffer[i].ElemCount > 0 && clip.Contains(ImRect(c.x, c.y, c.z, c.w)))
        {
            first_cmd = i;
            break;
        }
    }
    TranslateDrawList(dl, first_cmd, offset, clip);
    TranslateChildren(w, offset, clip);
}

//-----------------------------------------------------------------------------
// [SECTION] Let go, release (the pinch)
//-----------------------------------------------------------------------------

bool TouchScrollLetGo(bool even_awidget)
{
    ImGuiContext& g = *GImGui;
    ImGuiTouchScrollState& s = GetTouchContext().Scroll;
    const ImGuiID id = SentinelId();
    s.Inertia = ImVec2(0.0f, 0.0f);
    s.Pull = 0.0f;
    if (s.Owning)
    {
        s.Swiping = false;
        s.Parked = true;
        return true;
    }
    if (g.ActiveId == 0 || g.ActiveIdWindow == nullptr)
        return true;
    if (!even_awidget)
        return false;
    ImGui::SetActiveID(id, g.ActiveIdWindow);  // over the widget's id: it sees it lost the press
    s.Owning = true;
    s.Parked = true;
    return true;
}

bool TouchScrollRelease(bool even_awidget)
{
    ImGuiContext& g = *GImGui;
    ImGuiTouchScrollState& s = GetTouchContext().Scroll;
    const ImGuiID id = SentinelId();
    s.Inertia = ImVec2(0.0f, 0.0f);
    s.WatchingLongPress = s.LongPressArmed = false;
    if (s.Owning)
    {
        if (g.ActiveId == id)
            ImGui::ClearActiveID();
        EndPress(s);
        return true;
    }
    if (g.ActiveId == 0)
        return true;
    if (!even_awidget)
        return false;
    ImGui::ClearActiveID();
    return true;
}

//-----------------------------------------------------------------------------
// [SECTION] The scroll bar held by a finger
//-----------------------------------------------------------------------------

// A thin scroll bar held by a finger (the hold handed it the press) widens over the content, as on iOS: its thumb is
// drawn again, wider, where ImGui::ScrollbarEx() placed it. The bar itself keeps its width: ImGui reserves it in the
// window's layout, and the content would reflow under the finger.
void DrawTouchScrollbarGrab()
{
    ImGuiContext& g = *GImGui;
    ImGuiTouchScrollState& s = GetTouchContext().Scroll;
    ImGuiWindow* w = g.ActiveIdWindow;
    ImGuiAxis axis = ImGuiAxis_None;
    if (s.ThinScrollbars && w != nullptr && g.ActiveId != 0)
    {
        if (w->ScrollbarY && g.ActiveId == ImGui::GetWindowScrollbarID(w, ImGuiAxis_Y))
            axis = ImGuiAxis_Y;
        else if (w->ScrollbarX && g.ActiveId == ImGui::GetWindowScrollbarID(w, ImGuiAxis_X))
            axis = ImGuiAxis_X;
    }
    if (axis == ImGuiAxis_None)
    {
        s.HeldBar = 0;
        return;
    }
    if (s.HeldBar != g.ActiveId)
    {
        s.HeldBar = g.ActiveId;
        s.HeldBarTime = (float)g.Time;
    }

    // The thumb, as ScrollbarEx() places it (the sizes and the scroll truncated, as Scrollbar() passes them)
    const ImGuiStyle& style = g.Style;
    ImRect bb = ImGui::GetWindowScrollbarRect(w, axis);
    bb.Expand(-IM_TRUNC(ImMin(style.ScrollbarPadding, ImMin(bb.GetWidth(), bb.GetHeight()) * 0.5f)));
    const float bar_length = (axis == ImGuiAxis_Y) ? bb.GetHeight() : bb.GetWidth();
    const float visible = IM_TRUNC(w->InnerRect.Max[axis] - w->InnerRect.Min[axis]);
    const float contents = IM_TRUNC(w->ContentSize[axis] + w->WindowPadding[axis] * 2.0f);
    const float total = ImMax(ImMax(contents, visible), 1.0f);
    const float thumb_length =
        (float)(int)ImClamp(bar_length * visible / total, ImMin(bar_length, style.GrabMinSize), bar_length);
    const float scroll_ratio = ImSaturate(IM_TRUNC(w->Scroll[axis]) / ImMax(1.0f, contents - visible));
    const float thumb_start = bb.Min[axis] + scroll_ratio * (bar_length - thumb_length);

    // Wider, toward the content, growing for a moment. A few widths long at least (the thumb of a long list is short:
    // it would be a dot), around ImGui's thumb
    const float t = ImSaturate(((float)g.Time - s.HeldBarTime) / HELD_SCROLLBAR_GROW_SECONDS);
    const float thin = (axis == ImGuiAxis_Y) ? bb.GetWidth() : bb.GetHeight();
    const float width = ImLerp(thin, g.FontSize * HELD_SCROLLBAR_FONT_SIZES, t * (2.0f - t));  // eased out
    const float length = ImMin(ImMax(thumb_length, width * 2.5f), bar_length);
    const float start = ImClamp(thumb_start + (thumb_length - length) * 0.5f, bb.Min[axis], bb.Max[axis] - length);
    ImRect thumb = (axis == ImGuiAxis_Y) ? ImRect(bb.Max.x - width, start, bb.Max.x, start + length)
                                         : ImRect(start, bb.Max.y - width, start + length, bb.Max.y);

    // Opaque (the theme's color over the window's background): the content and the thin thumb do not show through
    ImVec4 color = ImLerp(style.Colors[ImGuiCol_WindowBg], style.Colors[ImGuiCol_ScrollbarGrabActive],
                          style.Colors[ImGuiCol_ScrollbarGrabActive].w);
    color.w = 1.0f;
    ImDrawList* draw_list = ImGui::GetForegroundDrawList(w->Viewport);
    draw_list->PushClipRect(w->Rect().Min, w->Rect().Max);
    draw_list->AddRectFilled(thumb.Min, thumb.Max, ImGui::GetColorU32(color), width * 0.5f);
    draw_list->PopClipRect();
    if (t < 1.0f)
        RequestAnimation();
}

//-----------------------------------------------------------------------------
// [SECTION] The widgets that take the drags
//-----------------------------------------------------------------------------

// The widget just drawn: its visible part, noted for the press of the next frame. Nothing when the layer did not run
// this frame (ImGuiTouch::Setup() not called): the notes would pile up, never read.
void AddDragTaker(const ImRect& item_rect, bool long_press_is_right_click, ImGuiAxis axis)
{
    ImGuiContext& g = *GImGui;
    ImGuiTouchContext* touch = FindTouchContext();  // none without the touch layer: the widget just draws
    if (touch == nullptr)
        return;
    ImGuiTouchScrollState& s = touch->Scroll;
    if (s.Context != &g || s.FrameCount != g.FrameCount)
        return;
    ImGuiWindow* window = g.CurrentWindow;
    ImRect rect = item_rect;
    rect.ClipWith(window->ClipRect);
    if (rect.GetWidth() > 0.0f && rect.GetHeight() > 0.0f)
    {
        ImGuiTouchScrollState::DragTaker taker = {window->ID, rect, long_press_is_right_click, axis};
        s.Takers.push_back(taker);
    }
}

void SetItemTakesDrags(ImGuiTouchAxis axis, bool long_press_is_right_click)
{
    const ImGuiAxis along =
        (axis == ImGuiTouchAxis_X) ? ImGuiAxis_X : (axis == ImGuiTouchAxis_Y) ? ImGuiAxis_Y : ImGuiAxis_None;
    AddDragTaker(GImGui->LastItemData.Rect, long_press_is_right_click, along);
}

}  // namespace ImGuiTouch
