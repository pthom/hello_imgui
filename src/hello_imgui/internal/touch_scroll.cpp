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
namespace HelloImGui
{
namespace
{
    // Tunables. The slop is in font sizes, so that it follows the DPI and the font scale.
    constexpr float kSlopFontSizes = 0.5f;      // a press that moved less than this is a tap, not a swipe
    constexpr float kHoldSeconds = 0.18f;       // a finger still for this long hands the press to the widget under it
    constexpr float kInertiaDecay = 4.f;        // speed *= exp(-decay * dt) after the release: a flick lasts about a second
    constexpr float kInertiaMinSpeed = 50.f;    // px/s: below this, a release starts no inertia, and the inertia ends
    constexpr float kVelocitySmoothing = 15.f;  // per second: the finger speed is averaged over the last ~70 ms

    struct State
    {
        bool owning = false;            // the sentinel holds the active id: a press, not released nor handed over yet
        bool swiping = false;           // the finger moved past the slop
        ImGuiWindow* window = nullptr;  // the pressed window, then the one that scrolls
        ImGuiAxis axis = ImGuiAxis_None;
        ImVec2 pressPos;
        ImVec2 velocity;                // of the finger, px/s, smoothed
        ImVec2 inertia;                 // px/s, after the release
        int replayedPresses = 0;        // presses queued by the layer, which it must not claim
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

    void EndPress(State& s)
    {
        s.owning = false;
        s.swiping = false;
    }

    // A replayed button event. The test engine erases, each frame, the queued events it did not add itself (the
    // backend's): these ones are ImGui's own, not the backend's, so they are marked as the engine marks its own.
    void AddReplayedButtonEvent(ImGuiIO& io, bool down)
    {
        ImGuiContext& g = *GImGui;
        int sizeBefore = g.InputEventsQueue.Size;
        io.AddMouseButtonEvent(ImGuiMouseButton_Left, down);
        if (g.InputEventsQueue.Size > sizeBefore)
            g.InputEventsQueue.back().AddedByTestEngine = true;
    }

    // The press goes to the widget under the finger: a release then a press, through the queue (two frames, with the
    // trickling rules), which the layer must not claim again
    void ReplayPress(ImGuiIO& io, State& s, bool fingerDown)
    {
        if (fingerDown)
            AddReplayedButtonEvent(io, false);
        AddReplayedButtonEvent(io, true);
        if (!fingerDown)
            AddReplayedButtonEvent(io, false);
        s.replayedPresses++;
    }
}  // namespace

void UpdateTouchScroll(TouchScrollMode mode)
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    State& s = gState;
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
            ImGuiWindow* w = g.HoveredWindow;
            if (w != nullptr && !w->Collapsed && ImGui::IsMousePosValid() && w->InnerRect.Contains(io.MousePos))
            {
                // A widget active from before (a text input being edited) loses the id, as with a press elsewhere
                ImGui::SetActiveID(id, w);
                ImGui::FocusWindow(w);  // what the press would have done
                s.owning = true;
                s.window = w;
                s.pressPos = io.MousePos;
                s.velocity = ImVec2(0.f, 0.f);
            }
        }
    }

    if (s.owning && g.ActiveId != id)  // a widget took the press itself
        EndPress(s);

    if (s.owning && !io.MouseDown[ImGuiMouseButton_Left])  // the release: a flick keeps scrolling, a tap is replayed
    {
        ImGui::ClearActiveID();
        bool flick = s.swiping && ImLengthSqr(s.velocity) > kInertiaMinSpeed * kInertiaMinSpeed;
        if (!s.swiping)
            ReplayPress(io, s, false);
        s.inertia = flick ? s.velocity : ImVec2(0.f, 0.f);
        EndPress(s);
    }

    if (s.owning)
    {
        ImGui::KeepAliveID(id);
        ImVec2 delta = io.MouseDelta;
        if (!s.swiping)
        {
            ImVec2 fromPress = io.MousePos - s.pressPos;
            float slop = g.FontSize * kSlopFontSizes;
            if (ImLengthSqr(fromPress) > slop * slop)
            {
                s.swiping = true;
                s.axis = SwipeAxis(fromPress);
                s.window = ScrollTarget(s.window, s.axis);
                delta = fromPress;  // the content catches up with the finger
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
            s.velocity = ImLerp(s.velocity, delta / dt, ImMin(1.f, dt * kVelocitySmoothing));
        }
    }

    if (s.inertia.x != 0.f || s.inertia.y != 0.f)
    {
        bool moving = ScrollBy(s.window, s.axis, Along(s.inertia, s.axis) * dt);
        s.inertia = s.inertia * std::exp(-kInertiaDecay * dt);
        if (!moving || ImLengthSqr(s.inertia) < kInertiaMinSpeed * kInertiaMinSpeed)
            s.inertia = ImVec2(0.f, 0.f);
    }
}

}  // namespace HelloImGui
