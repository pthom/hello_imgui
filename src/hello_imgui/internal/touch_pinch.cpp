#define IMGUI_DEFINE_MATH_OPERATORS  // before any include of imgui.h (runner_params.h includes it)
#include "hello_imgui/internal/touch_pinch.h"
#include "hello_imgui/internal/touch_scroll.h"

#include "imgui.h"
#include "imgui_internal.h"  // ImClamp, the input queue

namespace HelloImGui
{
namespace
{
    constexpr float kFontScaleMin = 0.5f, kFontScaleMax = 5.f;  // the range of the explorer's "Font scale" slider
    constexpr float kPinchThreshold = 0.12f;  // the distance between the fingers changed by this much: a pinch
    constexpr float kDragSlopFontSizes = 0.6f;  // the fingers' middle moved this far: a drag (the two apart)

    enum class Gesture { None, Undecided, Pinch, Drag };

    struct State
    {
        int count = 0;
        float pinchScale = 1.f;
        ImVec2 move;
        Gesture gesture = Gesture::None;
        float baseScale = 1.f;  // style.FontScaleMain when the pinch began
    };
    State gState;

    // The right button's events, marked as the test engine marks its own (see the swipe layer), with the mouse
    // source: the touch trickling would separate them from the positions
    void RightButton(ImGuiIO& io, bool down)
    {
        ImGuiContext& g = *GImGui;
        ImGuiMouseSource source = io.MouseSource;
        io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
        int before = g.InputEventsQueue.Size;
        if (down)
            io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);  // the first finger's press: no left drag with the right one
        if (g.InputEventsQueue.Size > before)
            g.InputEventsQueue.back().AddedByTestEngine = true;
        before = g.InputEventsQueue.Size;
        io.AddMouseButtonEvent(ImGuiMouseButton_Right, down);
        if (g.InputEventsQueue.Size > before)
            g.InputEventsQueue.back().AddedByTestEngine = true;
        io.AddMouseSourceEvent(source);
    }
}  // namespace

void SetTouchPointers(int count, float pinchScale, ImVec2 twoFingerMove)
{
    gState.count = count;
    gState.pinchScale = pinchScale;
    gState.move = twoFingerMove;
}

void UpdateTouchPinch(TouchPinchMode mode, bool interruptsWidgets)
{
    ImGuiContext& g = *GImGui;
    State& s = gState;
    bool twoFingers = (mode != TouchPinchMode::Disabled) && s.count >= 2;

    if (twoFingers && s.gesture == Gesture::None)
    {
        // The first finger was a press for the swipe layer, or for a widget: the two fingers take it
        if (!TouchScrollLetGo(interruptsWidgets))
            return;  // a widget keeps its drag: nothing with two fingers this time
        s.gesture = Gesture::Undecided;
    }
    if (s.gesture == Gesture::Undecided && twoFingers)
    {
        float slop = g.FontSize * kDragSlopFontSizes;
        if (ImFabs(s.pinchScale - 1.f) > kPinchThreshold)
        {
            s.gesture = Gesture::Pinch;
            s.baseScale = ImGui::GetStyle().FontScaleMain;
        }
        else if (ImLengthSqr(s.move) > slop * slop)
        {
            // A right drag: the swipe layer lets the widgets see the mouse again, the first finger's position
            // drives it (it is the mouse), the right button goes down
            s.gesture = Gesture::Drag;
            TouchScrollRelease(true);
            RightButton(g.IO, true);
        }
    }
    if (s.gesture == Gesture::Pinch && twoFingers && mode == TouchPinchMode::FontScale)
        ImGui::GetStyle().FontScaleMain = ImClamp(s.baseScale * s.pinchScale, kFontScaleMin, kFontScaleMax);
    if (s.gesture != Gesture::None && !twoFingers)
    {
        if (s.gesture == Gesture::Drag)
            RightButton(g.IO, false);
        s.gesture = Gesture::None;
    }
}

}  // namespace HelloImGui
