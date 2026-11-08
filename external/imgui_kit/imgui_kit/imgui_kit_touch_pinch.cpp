// imgui_kit: the pinch of a touch screen (ImGuiTouch)
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_kit.h"
#include "imgui_kit_touch_internal.h"

#include "imgui.h"
#include "imgui_internal.h"  // ImClamp, the input queue

namespace ImGuiTouch
{
namespace
{
    constexpr float kPinchThreshold = 0.12f;  // the distance between the fingers changed by this much: a pinch
    constexpr float kDragSlopFontSizes = 0.6f;  // the fingers' middle moved this far: a drag (the two apart)

    using Gesture = ImGuiTouchPinchGesture;
    using Finger = ImGuiTouchFinger;
    using State = ImGuiTouchPinchState;
    using Taker = ImGuiTouchPinchState::PinchTaker;

    float Distance(const Finger& a, const Finger& b) { return ImSqrt(ImLengthSqr(b.pos - a.pos)); }
    ImVec2 Center(const Finger& a, const Finger& b) { return (a.pos + b.pos) * 0.5f; }

    // A button or a wheel event, marked as the test engine marks its own (see the swipe layer), with the mouse
    // source: the touch trickling would separate a button from the positions
    void MarkLastEvent(ImGuiContext& g, int sizeBefore)
    {
        if (g.InputEventsQueue.Size > sizeBefore)
            g.InputEventsQueue.back().AddedByTestEngine = true;
    }
    void ButtonEvent(ImGuiIO& io, ImGuiMouseButton button, bool down)
    {
        ImGuiContext& g = *GImGui;
        ImGuiMouseSource source = io.MouseSource;
        io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
        int before = g.InputEventsQueue.Size;
        io.AddMouseButtonEvent(button, down);
        MarkLastEvent(g, before);
        io.AddMouseSourceEvent(source);
    }
    void WheelEvent(ImGuiIO& io, float notches)
    {
        ImGuiContext& g = *GImGui;
        ImGuiMouseSource source = io.MouseSource;
        io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
        int before = g.InputEventsQueue.Size;
        io.AddMouseWheelEvent(0.f, notches);
        MarkLastEvent(g, before);
        io.AddMouseSourceEvent(source);
    }

    // The widget that takes the pinch: both fingers inside a rectangle noted at the last frame (the smallest, when
    // several hold them: the innermost widget)
    const Taker* FindTaker(const State& s)
    {
        const Taker* found = nullptr;
        for (const Taker& t : s.previousTakers)
            if (t.rect.Contains(s.fingers[0].pos) && t.rect.Contains(s.fingers[1].pos))
                if (found == nullptr || t.rect.GetArea() < found->rect.GetArea())
                    found = &t;
        return found;
    }

    void AddTaker(const ImRect& rect, float zoomPerNotch, ImGuiMouseButton panButton)
    {
        ImGuiContext& g = *GImGui;
        ImGuiTouchContext* touch = FindCurrentContext();  // none without the touch layer: the widget just draws
        if (touch == nullptr || touch->Pinch.frameCount != g.FrameCount)
            return;
        ImRect visible = rect;
        visible.ClipWith(g.CurrentWindow->ClipRect);
        if (visible.GetWidth() <= 0.f || visible.GetHeight() <= 0.f || zoomPerNotch <= 0.f)
            return;
        Taker taker = {visible, zoomPerNotch, panButton};
        touch->Pinch.takers.push_back(taker);
    }
}  // namespace

void SetItemTakesPinch(float zoomPerNotch, ImGuiMouseButton panButton)
{
    AddTaker(GImGui->LastItemData.Rect, zoomPerNotch, panButton);
}

void SetRectTakesPinch(const ImVec2& min, const ImVec2& max, float zoomPerNotch, ImGuiMouseButton panButton)
{
    AddTaker(ImRect(min, max), zoomPerNotch, panButton);
}

void AddFingerEvent(int id, const ImVec2& pos, bool down)
{
    State& s = GetCurrentContext().Pinch;
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
    State& s = GetCurrentContext().Pinch;
    s.previousTakers.swap(s.takers);
    s.takers.resize(0);
    s.frameCount = g.FrameCount;
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
        const Taker* taker = FindTaker(s);
        s.hasTaker = (taker != nullptr);
        if (s.hasTaker)
        {
            // A widget takes the pinch: the swipe layer lets go for good, and the first finger's press is released,
            // so that the widget is hovered (it reads the wheel then), and is not dragged by it
            s.taker = *taker;
            s.notchesSent = 0;
            if (!TouchScrollRelease(interruptsWidgets))
                return;  // a widget keeps its drag
            ButtonEvent(g.IO, ImGuiMouseButton_Left, false);
        }
        // The first finger was a press for the swipe layer, or for a widget: the two fingers take it
        else if (!TouchScrollLetGo(interruptsWidgets))
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
            // A right drag (or the taker's pan button): the swipe layer lets the widgets see the mouse again, the
            // first finger's press is released, and the button goes down a frame later (a plot cancels a box
            // selection started in the frame of a left release); the first finger's position drives the drag
            s.gesture = Gesture::DragStarting;
            if (!s.hasTaker)  // a taker's: released when the fingers landed
            {
                TouchScrollRelease(true);
                ButtonEvent(g.IO, ImGuiMouseButton_Left, false);
            }
        }
    }
    else if (s.gesture == Gesture::DragStarting && twoFingers)
    {
        s.gesture = Gesture::Drag;
        const ImGuiMouseButton button = s.hasTaker ? s.taker.panButton : ImGuiMouseButton_Right;
        if (button == ImGuiMouseButton_Left)
            GetCurrentContext().Scroll.replayedPresses++;  // the layer's own press: the swipe does not claim it
        ButtonEvent(g.IO, button, true);
    }
    if (s.gesture == Gesture::Pinch && twoFingers && s.hasTaker)
    {
        // The taker zooms by wheel notches: one each time the spread changed by its zoom per notch, at most one per
        // frame (the notches of one frame add up in io.MouseWheel, and a plot zooms once per frame)
        const float target = ImLog(pinchScale) / ImLog(1.f + s.taker.zoomPerNotch);
        const float missing = target - (float)s.notchesSent;
        const int notch = (missing >= 1.f) ? 1 : (missing <= -1.f) ? -1 : 0;
        if (notch != 0)
        {
            WheelEvent(g.IO, (float)notch);
            s.notchesSent += notch;
        }
    }
    else if (s.gesture == Gesture::Pinch && twoFingers && mode == ImGuiTouchPinchMode_FontScale)
    {
        const ImGuiTouchConfig& config = GetCurrentContext().Config;
        ImGui::GetStyle().FontScaleMain = ImClamp(s.baseScale * pinchScale, config.PinchFontScaleMin, config.PinchFontScaleMax);
    }
    if (s.gesture != Gesture::None && !twoFingers)
    {
        if (s.gesture == Gesture::Drag)
            ButtonEvent(g.IO, s.hasTaker ? s.taker.panButton : ImGuiMouseButton_Right, false);
        s.gesture = Gesture::None;
        s.hasTaker = false;
    }
}

}  // namespace ImGuiTouch
