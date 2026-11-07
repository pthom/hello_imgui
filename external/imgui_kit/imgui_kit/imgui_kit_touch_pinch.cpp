// imgui_kit: the pinch of a touch screen (ImGuiTouch)
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_kit.h"
#include "imgui_kit_internal.h"

#include "imgui.h"
#include "imgui_internal.h"  // ImClamp, the input queue

namespace ImGuiTouch
{
namespace
{
    constexpr float kFontScaleMin = 0.5f, kFontScaleMax = 5.f;  // the range of the explorer's "Font scale" slider
    constexpr float kPinchThreshold = 0.12f;  // the distance between the fingers changed by this much: a pinch
    constexpr float kDragSlopFontSizes = 0.6f;  // the fingers' middle moved this far: a drag (the two apart)

    enum class Gesture { None, Undecided, Pinch, DragStarting, Drag };

    struct Finger
    {
        int id;
        ImVec2 pos;
    };

    struct State
    {
        ImVector<Finger> fingers;  // the fingers down, in the order they landed
        float startDistance = 0.f;  // between the first two fingers, when the second one landed
        ImVec2 startCenter;         // their middle, then
        Gesture gesture = Gesture::None;
        float baseScale = 1.f;  // style.FontScaleMain when the pinch began
    };
    State gState;

    float Distance(const Finger& a, const Finger& b) { return ImSqrt(ImLengthSqr(b.pos - a.pos)); }
    ImVec2 Center(const Finger& a, const Finger& b) { return (a.pos + b.pos) * 0.5f; }

    // A button event, marked as the test engine marks its own (see the swipe layer), with the mouse source: the
    // touch trickling would separate it from the positions
    void ButtonEvent(ImGuiIO& io, ImGuiMouseButton button, bool down)
    {
        ImGuiContext& g = *GImGui;
        ImGuiMouseSource source = io.MouseSource;
        io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
        int before = g.InputEventsQueue.Size;
        io.AddMouseButtonEvent(button, down);
        if (g.InputEventsQueue.Size > before)
            g.InputEventsQueue.back().AddedByTestEngine = true;
        io.AddMouseSourceEvent(source);
    }
}  // namespace

void AddFingerEvent(int id, const ImVec2& pos, bool down)
{
    State& s = gState;
    bool wasTwo = s.fingers.Size >= 2;
    int index = -1;
    for (int i = 0; i < s.fingers.Size; i++)
        if (s.fingers[i].id == id)
            index = i;
    if (!down)
    {
        if (index >= 0)
            s.fingers.erase(s.fingers.Data + index);
        return;
    }
    if (index >= 0)
        s.fingers[index].pos = pos;
    else
        s.fingers.push_back({id, pos});
    if (!wasTwo && s.fingers.Size >= 2)  // the second finger landed: the pinch and the drag measure from here
    {
        s.startDistance = Distance(s.fingers[0], s.fingers[1]);
        s.startCenter = Center(s.fingers[0], s.fingers[1]);
    }
}

void UpdateTouchPinch(ImGuiTouchPinchMode mode, bool interruptsWidgets)
{
    ImGuiContext& g = *GImGui;
    State& s = gState;
    bool twoFingers = (mode != ImGuiTouchPinchMode_Disabled) && s.fingers.Size >= 2;
    // The pinch's scale (their distance over their distance at the start), the move of their middle since the start
    float pinchScale = 1.f;
    ImVec2 move;
    if (s.fingers.Size >= 2)
    {
        if (s.startDistance > 0.f)
            pinchScale = Distance(s.fingers[0], s.fingers[1]) / s.startDistance;
        move = Center(s.fingers[0], s.fingers[1]) - s.startCenter;
    }

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
        if (ImFabs(pinchScale - 1.f) > kPinchThreshold)
        {
            s.gesture = Gesture::Pinch;
            s.baseScale = ImGui::GetStyle().FontScaleMain;
        }
        else if (ImLengthSqr(move) > slop * slop)
        {
            // A right drag: the swipe layer lets the widgets see the mouse again, the first finger's press is
            // released, and the right button goes down a frame later (a plot cancels a box selection started
            // in the frame of a left release); the first finger's position drives the drag (it is the mouse)
            s.gesture = Gesture::DragStarting;
            TouchScrollRelease(true);
            ButtonEvent(g.IO, ImGuiMouseButton_Left, false);
        }
    }
    else if (s.gesture == Gesture::DragStarting && twoFingers)
    {
        s.gesture = Gesture::Drag;
        ButtonEvent(g.IO, ImGuiMouseButton_Right, true);
    }
    if (s.gesture == Gesture::Pinch && twoFingers && mode == ImGuiTouchPinchMode_FontScale)
        ImGui::GetStyle().FontScaleMain = ImClamp(s.baseScale * pinchScale, kFontScaleMin, kFontScaleMax);
    if (s.gesture != Gesture::None && !twoFingers)
    {
        if (s.gesture == Gesture::Drag)
            ButtonEvent(g.IO, ImGuiMouseButton_Right, false);
        s.gesture = Gesture::None;
    }
}

}  // namespace ImGuiTouch
