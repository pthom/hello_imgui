#define IMGUI_DEFINE_MATH_OPERATORS  // before any include of imgui.h (runner_params.h includes it)
#include "hello_imgui/internal/touch_scroll.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <cmath>

// The swipe, as a layer above the backends and below the widgets:
// - a press that no widget took (the void of a window, a text) is claimed with a sentinel active id, at the end of
//   the frame. This stops ImGui from moving the window, and the widgets from activating under the moving finger.
//   A widget that wants a long press (a text selection) takes the active id after its delay: the layer then steps
//   aside. (The ownership part of ocornut's snippet in https://github.com/ocornut/imgui/issues/3379.)
// - once the finger moved past a slop, each frame's motion becomes a mouse wheel event: ImGui's wheel path finds the
//   scrollable window (a child, else its parent), honours the no-scroll flags, and lets the widgets that eat the wheel
//   (plots) keep it. A tap (released before the slop) does nothing.
// - on the release, the speed of the finger becomes an inertia that decays.
namespace HelloImGui
{
namespace
{
    // Tunables. The slop is in font sizes, so that it follows the DPI and the font scale.
    constexpr float kSlopFontSizes = 0.5f;      // a press that moved less than this is a tap, not a swipe
    constexpr float kInertiaDecay = 4.f;        // speed *= exp(-decay * dt) after the release: a flick lasts about a second
    constexpr float kInertiaMinSpeed = 50.f;    // px/s: below this, a release starts no inertia, and the inertia ends
    constexpr float kVelocitySmoothing = 15.f;  // per second: the finger speed is averaged over the last ~70 ms

    struct State
    {
        bool owning = false;            // the sentinel holds the active id: a press on the void, not released yet
        bool swiping = false;           // the finger moved past the slop
        ImGuiWindow* window = nullptr;  // the window ImGui will scroll: it gives the wheel step
        ImGuiAxis axis = ImGuiAxis_None;
        ImVec2 pressPos;
        ImVec2 velocity;                // of the finger, px/s, smoothed
        ImVec2 inertia;                 // px/s, after the release
        bool trickleSaved = false;      // io.ConfigInputTrickleEventQueue before the swipe
    };
    State gState;

    ImGuiID SentinelId() { return ImHashStr("##HelloImGui_TouchScroll"); }

    // ImGui scrolls a window by scroll_step pixels per wheel unit (see UpdateMouseWheel): the inverse, for a pixel delta
    ImVec2 PixelsToWheel(const ImGuiWindow* w, ImVec2 px)
    {
        float stepX = ImTrunc(ImMin(2.f * w->FontRefSize, w->InnerRect.GetWidth() * 0.67f));
        float stepY = ImTrunc(ImMin(5.f * w->FontRefSize, w->InnerRect.GetHeight() * 0.67f));
        return ImVec2(stepX > 0.f ? px.x / stepX : 0.f, stepY > 0.f ? px.y / stepY : 0.f);
    }

    ImVec2 OnAxis(ImVec2 v, ImGuiAxis axis) { return axis == ImGuiAxis_X ? ImVec2(v.x, 0.f) : ImVec2(0.f, v.y); }

    // The axis of a swipe: the dominant direction of the finger (a swipe scrolls one axis, like the wheel)
    ImGuiAxis SwipeAxis(ImVec2 fromPress)
    {
        return (ImFabs(fromPress.x) > ImFabs(fromPress.y)) ? ImGuiAxis_X : ImGuiAxis_Y;
    }

    // The window that ImGui's wheel path will scroll on this axis (FindBestWheelingWindow): the pressed one, or the
    // first parent that can scroll that way. A child at the end of its scroll keeps the wheel, like with a mouse.
    ImGuiWindow* WheelTarget(ImGuiWindow* w, ImGuiAxis axis)
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

    // The finger's position and our wheel events cannot pass ImGui's input queue in the same frame: the trickling
    // rules defer one of them, and the content would follow the finger at half the frame rate. So the trickling is
    // off during a swipe (the press is behind us: the rule that matters for a touch, "no hover before a press",
    // has done its job), and restored when the finger lifts.
    void SetSwiping(ImGuiIO& io, State& s, bool swiping)
    {
        if (swiping == s.swiping)
            return;
        if (swiping)
        {
            s.trickleSaved = io.ConfigInputTrickleEventQueue;
            io.ConfigInputTrickleEventQueue = false;
        }
        else
            io.ConfigInputTrickleEventQueue = s.trickleSaved;
        s.swiping = swiping;
    }

    void EndPress(ImGuiIO& io, State& s)
    {
        SetSwiping(io, s, false);
        s.owning = false;
        s.axis = ImGuiAxis_None;
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
        EndPress(io, s);
        s.inertia = ImVec2(0.f, 0.f);
        return;
    }
    const float dt = (io.DeltaTime > 0.f) ? io.DeltaTime : 1.f / 60.f;

    // A press stops the inertia. A press on the content of a window, that no widget took, is ours.
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        s.inertia = ImVec2(0.f, 0.f);
        ImGuiWindow* w = g.HoveredWindow;
        if (g.ActiveId == 0 && w != nullptr && !w->Collapsed && ImGui::IsMousePosValid()
            && w->InnerRect.Contains(io.MousePos))
        {
            ImGui::SetActiveID(id, w);
            ImGui::FocusWindow(w);  // what the click would have done, had we not taken the active id
            s.owning = true;
            s.window = w;
            s.pressPos = io.MousePos;
            s.velocity = ImVec2(0.f, 0.f);
        }
    }

    if (s.owning && g.ActiveId != id)  // a widget took the press (a long press)
        EndPress(io, s);

    if (s.owning && !io.MouseDown[ImGuiMouseButton_Left])  // the release: a flick keeps scrolling
    {
        ImGui::ClearActiveID();
        bool flick = s.swiping && ImLengthSqr(s.velocity) > kInertiaMinSpeed * kInertiaMinSpeed;
        s.inertia = flick ? OnAxis(s.velocity, s.axis) : ImVec2(0.f, 0.f);
        EndPress(io, s);
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
                SetSwiping(io, s, true);
                s.axis = SwipeAxis(fromPress);
                s.window = WheelTarget(s.window, s.axis);
                delta = fromPress;  // the content catches up with the finger
            }
        }
        if (s.swiping)
        {
            ImVec2 d = OnAxis(delta, s.axis);
            ImVec2 wheel = PixelsToWheel(s.window, d);
            if (wheel.x != 0.f || wheel.y != 0.f)
                io.AddMouseWheelEvent(wheel.x, wheel.y);
            s.velocity = ImLerp(s.velocity, d / dt, ImMin(1.f, dt * kVelocitySmoothing));
        }
    }

    if (s.inertia.x != 0.f || s.inertia.y != 0.f)
    {
        ImVec2 wheel = PixelsToWheel(s.window, s.inertia * dt);
        io.AddMouseWheelEvent(wheel.x, wheel.y);
        s.inertia = s.inertia * std::exp(-kInertiaDecay * dt);
        if (ImLengthSqr(s.inertia) < kInertiaMinSpeed * kInertiaMinSpeed)
            s.inertia = ImVec2(0.f, 0.f);
    }
}

}  // namespace HelloImGui
