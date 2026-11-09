// imgui_kit: the pinch of a touch screen (ImGuiTouch)
// https://github.com/pthom/imgui_kit (MIT license)

/*

Index of this file:
// [SECTION] Helpers: the events, the takers
// [SECTION] The claims (ItemPinch, the adapters)
// [SECTION] The fingers (AddFingerEvent)
// [SECTION] UpdateTouchPinch

*/

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_kit.h"
#include "imgui_kit_touch_internal.h"

#include "imgui.h"
#include "imgui_internal.h"  // ImClamp, the input queue

namespace ImGuiTouch
{
//-----------------------------------------------------------------------------
// [SECTION] Helpers: the events, the takers
//-----------------------------------------------------------------------------

constexpr float PINCH_THRESHOLD = 0.12f;  // the distance between the fingers changed by this much: a pinch
constexpr float DRAG_SLOP_FONT_SIZES = 0.6f;  // the fingers' middle moved this far: a drag (the two apart)

static float Distance(const ImGuiTouchFinger& a, const ImGuiTouchFinger& b)
{
    return ImSqrt(ImLengthSqr(b.Pos - a.Pos));
}
static ImVec2 Center(const ImGuiTouchFinger& a, const ImGuiTouchFinger& b) { return (a.Pos + b.Pos) * 0.5f; }

// A button or a wheel event, marked as the test engine marks its own (see the swipe layer), with the mouse
// source: the touch trickling would separate a button from the positions
static void MarkLastEvent(ImGuiContext& g, int size_before)
{
    if (g.InputEventsQueue.Size > size_before)
        g.InputEventsQueue.back().AddedByTestEngine = true;
}
static void ButtonEvent(ImGuiIO& io, ImGuiMouseButton button, bool down)
{
    ImGuiContext& g = *GImGui;
    ImGuiMouseSource source = io.MouseSource;
    io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
    int before = g.InputEventsQueue.Size;
    io.AddMouseButtonEvent(button, down);
    MarkLastEvent(g, before);
    io.AddMouseSourceEvent(source);
}
static void PosEvent(ImGuiIO& io, ImVec2 pos)
{
    ImGuiContext& g = *GImGui;
    int before = g.InputEventsQueue.Size;
    io.AddMousePosEvent(pos.x, pos.y);
    MarkLastEvent(g, before);
}
static void WheelEvent(ImGuiIO& io, float notches)
{
    ImGuiContext& g = *GImGui;
    ImGuiMouseSource source = io.MouseSource;
    io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
    int before = g.InputEventsQueue.Size;
    io.AddMouseWheelEvent(0.0f, notches);
    MarkLastEvent(g, before);
    io.AddMouseSourceEvent(source);
}

// The widget that takes the pinch: both fingers inside a rectangle noted at the last frame (the smallest, when
// several hold them: the innermost widget)
const ImGuiTouchPinchState::PinchTaker* FindTaker(const ImGuiTouchPinchState& s)
{
    const ImGuiTouchPinchState::PinchTaker* found = nullptr;
    for (const ImGuiTouchPinchState::PinchTaker& t : s.PreviousTakers)
        if (t.Rect.Contains(s.Fingers[0].Pos) && t.Rect.Contains(s.Fingers[1].Pos))
            if (found == nullptr || t.Rect.GetArea() < found->Rect.GetArea())
                found = &t;
    return found;
}

// The claim of a widget at this frame: an item's claims merge by its id, a rectangle's (id 0) by the rectangle.
// Null without the touch layer (the widget just draws), or when nothing of the widget shows.
static ImGuiTouchPinchState::PinchTaker* Claim(ImGuiID id, const ImRect& rect)
{
    ImGuiContext& g = *GImGui;
    ImGuiTouchContext* touch = FindTouchContext();
    if (touch == nullptr || touch->Pinch.FrameCount != g.FrameCount)
        return nullptr;
    ImRect visible = rect;
    visible.ClipWith(g.CurrentWindow->ClipRect);
    if (visible.GetWidth() <= 0.0f || visible.GetHeight() <= 0.0f)
        return nullptr;
    ImGuiTouchPinchState& s = touch->Pinch;
    for (ImGuiTouchPinchState::PinchTaker& t : s.Takers)
    {
        const bool same = (id != 0) ? (t.Id == id)
                                    : (t.Id == 0 && t.Rect.Min == visible.Min && t.Rect.Max == visible.Max);
        if (same)
            return &t;
    }
    ImGuiTouchPinchState::PinchTaker t;
    t.Id = id;
    t.Rect = visible;
    s.Takers.push_back(t);
    return &s.Takers.back();
}

static void ClaimWheel(ImGuiID id, const ImRect& rect, float wheel_zoom_factor, ImGuiTouchPinchDelivery delivery)
{
    IM_ASSERT(wheel_zoom_factor > 0.0f && wheel_zoom_factor != 1.0f && "a zoom per wheel unit, e.g. 1.1 for +10%");
    if (ImGuiTouchPinchState::PinchTaker* t = Claim(id, rect))
    {
        IM_ASSERT(!t->Data && "a widget gets the pinch's data (ItemPinch) or wheel events, not both");
        t->Wheel = true;
        t->WheelZoomFactor = wheel_zoom_factor;
        t->Delivery = delivery;
    }
}

static void ClaimPan(ImGuiID id, const ImRect& rect, ImGuiMouseButton button)
{
    if (ImGuiTouchPinchState::PinchTaker* t = Claim(id, rect))
    {
        IM_ASSERT(!t->Data && "a widget gets the pinch's data (ItemPinch) or a mouse drag, not both");
        t->Pan = true;
        t->PanButton = button;
    }
}

// The mouse button that two fingers moved together press: none for a widget that reads the gesture's data
static bool DragButton(const ImGuiTouchPinchState& s, ImGuiMouseButton* button)
{
    if (s.HasTaker && s.Taker.Data)
        return false;
    *button = (s.HasTaker && s.Taker.Pan) ? s.Taker.PanButton : ImGuiMouseButton_Right;
    return true;
}

// The fingers lifted (or were cancelled): the drag's button lifts, and a widget that reads the data sees Ended
static void EndGesture(ImGuiContext& g, ImGuiTouchPinchState& s)
{
    ImGuiMouseButton button;
    if (s.Gesture == ImGuiTouchPinchGesture_Drag && DragButton(s, &button))
        ButtonEvent(g.IO, button, false);
    if (s.HasTaker && s.Taker.Data)
    {
        s.Data.Active = s.Data.Started = false;
        s.Data.Ended = true;
        s.Data.CenterDelta = ImVec2(0.0f, 0.0f);
        s.Data.ScaleDelta = 1.0f;
        s.EndedId = s.Taker.Id;
        s.EndedFrame = g.FrameCount;
    }
    s.Gesture = ImGuiTouchPinchGesture_None;
    s.HasTaker = false;
}

static ImGuiTouchPinch DataFor(ImGuiID id)
{
    ImGuiTouchContext* touch = FindTouchContext();
    if (touch == nullptr || id == 0)
        return ImGuiTouchPinch();
    const ImGuiTouchPinchState& s = touch->Pinch;
    const bool active = s.HasTaker && s.Taker.Data && s.Taker.Id == id && s.Gesture != ImGuiTouchPinchGesture_None;
    const bool ended = s.EndedId == id && s.EndedFrame == GImGui->FrameCount;
    return (active || ended) ? s.Data : ImGuiTouchPinch();
}

//-----------------------------------------------------------------------------
// [SECTION] The claims (ItemPinch, the adapters)
//-----------------------------------------------------------------------------

ImGuiTouchPinch ItemPinch()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.LastItemData.ID != 0 && "ItemPinch() needs an item with an id (an InvisibleButton, not an Image)");
    return RectPinch(g.LastItemData.ID, g.LastItemData.Rect.Min, g.LastItemData.Rect.Max);
}

ImGuiTouchPinch RectPinch(ImGuiID id, const ImVec2& min, const ImVec2& max)
{
    if (ImGuiTouchPinchState::PinchTaker* t = Claim(id, ImRect(min, max)))
    {
        IM_ASSERT(!t->Wheel && !t->Pan && "a widget gets the pinch's data (ItemPinch) or the adapters, not both");
        t->Data = true;
    }
    return DataFor(id);
}

void SetItemPinchAsWheel(float wheel_zoom_factor, ImGuiTouchPinchDelivery delivery)
{
    ClaimWheel(GImGui->LastItemData.ID, GImGui->LastItemData.Rect, wheel_zoom_factor, delivery);
}

void SetRectPinchAsWheel(const ImVec2& min, const ImVec2& max, float wheel_zoom_factor,
                         ImGuiTouchPinchDelivery delivery)
{
    ClaimWheel(0, ImRect(min, max), wheel_zoom_factor, delivery);
}

void SetItemTwoFingerPanAsMouseDrag(ImGuiMouseButton button)
{
    ClaimPan(GImGui->LastItemData.ID, GImGui->LastItemData.Rect, button);
}

void SetRectTwoFingerPanAsMouseDrag(const ImVec2& min, const ImVec2& max, ImGuiMouseButton button)
{
    ClaimPan(0, ImRect(min, max), button);
}

//-----------------------------------------------------------------------------
// [SECTION] The fingers (AddFingerEvent)
//-----------------------------------------------------------------------------

void AddFingerEvent(int id, const ImVec2& pos, ImGuiTouchPhase phase)
{
    ImGuiTouchPinchState& s = GetTouchContext().Pinch;
    const bool down = (phase == ImGuiTouchPhase_Began || phase == ImGuiTouchPhase_Moved);
    if (phase == ImGuiTouchPhase_Cancelled)
        s.Cancelled = true;
    bool was_two = s.Fingers.Size >= 2;
    int index = -1;
    for (int i = 0; i < s.Fingers.Size; i++)
        if (s.Fingers[i].Id == id)
            index = i;
    if (!down)
    {
        if (index < 0)
            return;
        // One of the two fingers of the gesture lifts while a third is down: the third takes its place, and the
        // scale goes on from where it was (no jump), measured from the new pair
        const bool pair_changes = (index < 2 && s.Fingers.Size >= 3);
        const float scale_before = (pair_changes && s.StartDistance > 0.0f)
            ? s.ScaleBase * Distance(s.Fingers[0], s.Fingers[1]) / s.StartDistance : 1.0f;
        s.Fingers.erase(s.Fingers.Data + index);
        if (pair_changes)
        {
            s.ScaleBase = scale_before;
            s.StartDistance = Distance(s.Fingers[0], s.Fingers[1]);
            s.StartCenter = Center(s.Fingers[0], s.Fingers[1]);
            s.LastDistance = s.StartDistance;
            s.LastCenter = s.StartCenter;
        }
        return;
    }
    if (index >= 0)
        s.Fingers[index].Pos = pos;
    else
        s.Fingers.push_back({id, pos});
    if (!was_two && s.Fingers.Size >= 2)  // the second finger landed: the pinch and the drag measure from here
    {
        s.ScaleBase = 1.0f;
        s.StartDistance = Distance(s.Fingers[0], s.Fingers[1]);
        s.StartCenter = Center(s.Fingers[0], s.Fingers[1]);
    }
}

void ApplyFingerCancel()
{
    ImGuiContext& g = *GImGui;
    ImGuiTouchPinchState& s = GetTouchContext().Pinch;
    if (!s.Cancelled)
        return;
    s.Cancelled = false;
    if (s.Gesture != ImGuiTouchPinchGesture_None)
        EndGesture(g, s);
    TouchScrollRelease(true);  // the swipe's press, or the widget's: neither clicks when the mouse lifts
}

// While a widget's pinch lasts (or its pan), the mouse is the fingers' center, so that the widget zooms around it. The
// platform's moves (its mouse is the first finger) are replaced: the mouse goes to the center when it is half a line
// away from it, and stays still otherwise. A move holds the next wheel event back a frame (Dear ImGui reads a wheel
// event queued after a move at the next frame): a mouse that followed each small move would halve the zoom's pace.
// Events marked as the layer's or the Test Engine's are left as they are.
void PinchBeforeNewFrame()
{
    ImGuiContext& g = *GImGui;
    ImGuiTouchContext* touch = FindTouchContext();
    if (touch == nullptr)
        return;
    ImGuiTouchPinchState& s = touch->Pinch;
    if (!s.HasTaker || s.Gesture == ImGuiTouchPinchGesture_None || s.Fingers.Size < 2)
        return;
    const ImVec2 center = Center(s.Fingers[0], s.Fingers[1]);
    const float tolerance = g.FontSize * 0.5f;
    const bool far = ImLengthSqr(g.IO.MousePos - center) > tolerance * tolerance;
    bool moved = false;
    for (int i = 0; i < g.InputEventsQueue.Size;)
    {
        ImGuiInputEvent& e = g.InputEventsQueue[i];
        if (e.Type != ImGuiInputEventType_MousePos || e.AddedByTestEngine)
            ++i;
        else if (far && !moved)
        {
            e.MousePos.PosX = center.x;
            e.MousePos.PosY = center.y;
            moved = true;
            ++i;
        }
        else
            g.InputEventsQueue.erase(g.InputEventsQueue.Data + i);
    }
    if (far && !moved)
        PosEvent(g.IO, center);
}

//-----------------------------------------------------------------------------
// [SECTION] UpdateTouchPinch
//-----------------------------------------------------------------------------

void UpdateTouchPinch(bool scales_font)
{
    ImGuiContext& g = *GImGui;
    ImGuiTouchPinchState& s = GetTouchContext().Pinch;
    s.PreviousTakers.swap(s.Takers);
    s.Takers.resize(0);
    s.FrameCount = g.FrameCount;
    bool two_fingers = s.Fingers.Size >= 2;
    // The pinch's scale (their distance over their distance at the start), the move of their middle since the start
    float pinch_scale = 1.0f;
    float distance = 0.0f;
    ImVec2 center, move;
    if (two_fingers)
    {
        distance = Distance(s.Fingers[0], s.Fingers[1]);
        center = Center(s.Fingers[0], s.Fingers[1]);
        if (s.StartDistance > 0.0f)
            pinch_scale = s.ScaleBase * distance / s.StartDistance;
        move = center - s.StartCenter;
    }

    bool started = false;
    if (two_fingers && s.Gesture == ImGuiTouchPinchGesture_None)
    {
        const ImGuiTouchPinchState::PinchTaker* taker = FindTaker(s);
        s.HasTaker = (taker != nullptr);
        if (s.HasTaker)
        {
            // A widget claims the two fingers: the swipe layer lets go for good, and the first finger's press is
            // released, so that the widget is hovered (it reads the wheel then), and is not dragged by it. The widget's
            // own press (a canvas that took the first finger) is released too.
            s.Taker = *taker;
            s.NotchesSent = 0.0f;
            const bool its_own_press = (s.Taker.Id != 0 && g.ActiveId == s.Taker.Id);
            if (!TouchScrollRelease(its_own_press))
            {
                s.HasTaker = false;
                return;  // another widget keeps its drag
            }
            ButtonEvent(g.IO, ImGuiMouseButton_Left, false);
            PosEvent(g.IO, center);  // the mouse goes to the fingers' center
        }
        // The first finger was a press for the swipe layer, or for a widget: the two fingers take it
        else if (!TouchScrollLetGo(false))
            return;  // a widget keeps its drag: nothing with two fingers this time
        s.Gesture = ImGuiTouchPinchGesture_Undecided;
        started = true;
    }
    if (s.Gesture == ImGuiTouchPinchGesture_Undecided && two_fingers)
    {
        float slop = g.FontSize * DRAG_SLOP_FONT_SIZES;
        if (ImFabs(pinch_scale - 1.0f) > PINCH_THRESHOLD)
        {
            s.Gesture = ImGuiTouchPinchGesture_Pinch;
            s.BaseScale = ImGui::GetStyle().FontScaleMain;
            // A widget that reads the wheel's value zooms from here, without catching up the threshold at once
            if (s.HasTaker && s.Taker.Wheel && s.Taker.Delivery == ImGuiTouchPinchDelivery_WheelContinuous)
                s.NotchesSent = ImLog(pinch_scale) / ImLog(s.Taker.WheelZoomFactor);
        }
        else if (ImLengthSqr(move) > slop * slop)
        {
            // A drag with two fingers. Unless the widget reads the data: the swipe layer lets the widgets see the
            // mouse again, the first finger's press is released, and the button goes down a frame later (a plot
            // cancels a box selection started in the frame of a left release)
            s.Gesture = ImGuiTouchPinchGesture_DragStarting;
            if (!s.HasTaker)  // a taker's: released when the fingers landed
            {
                TouchScrollRelease(true);
                ButtonEvent(g.IO, ImGuiMouseButton_Left, false);
            }
        }
    }
    else if (s.Gesture == ImGuiTouchPinchGesture_DragStarting && two_fingers)
    {
        s.Gesture = ImGuiTouchPinchGesture_Drag;
        ImGuiMouseButton button;
        if (DragButton(s, &button))
        {
            if (button == ImGuiMouseButton_Left)
                GetTouchContext().Scroll.ReplayedPresses++;  // the layer's own press: the swipe does not claim it
            ButtonEvent(g.IO, button, true);
        }
    }
    if (s.Gesture == ImGuiTouchPinchGesture_Pinch && two_fingers && s.HasTaker)
    {
        if (s.Taker.Wheel)
        {
            // The taker zooms by the wheel: the wheel units its zoom factor gives for the fingers' spread
            const float target = ImLog(pinch_scale) / ImLog(s.Taker.WheelZoomFactor);
            const float missing = target - s.NotchesSent;
            if (s.Taker.Delivery == ImGuiTouchPinchDelivery_WheelContinuous)
            {
                // The exact amount at each frame: the zoom follows the fingers (amounts queued together add up exactly)
                if (ImFabs(missing) > 0.001f)
                {
                    WheelEvent(g.IO, missing);
                    s.NotchesSent += missing;
                }
            }
            else
            {
                // Whole notches, one at a time: a notch queued while another waits would be read with it, in the
                // same frame (Dear ImGui reads a wheel event queued after a move at the next frame, and the fingers'
                // moves keep coming), and their sum zooms a plot once: ImPlot reads only the wheel's sign
                bool notch_waiting = false;
                for (const ImGuiInputEvent& e : g.InputEventsQueue)
                    if (e.Type == ImGuiInputEventType_MouseWheel)
                        notch_waiting = true;
                const float notch = notch_waiting ? 0.0f : (missing >= 1.0f) ? 1.0f : (missing <= -1.0f) ? -1.0f : 0.0f;
                if (notch != 0.0f)
                {
                    WheelEvent(g.IO, notch);
                    s.NotchesSent += notch;
                }
            }
        }
        // A widget with the pan adapter only: a pinch on it does nothing; one that reads the data: below
    }
    else if (s.Gesture == ImGuiTouchPinchGesture_Pinch && two_fingers && scales_font)
    {
        const ImGuiTouchConfig& config = GetTouchContext().Config;
        ImGui::GetStyle().FontScaleMain =
            ImClamp(s.BaseScale * pinch_scale, config.PinchFontScaleMin, config.PinchFontScaleMax);
    }

    // The data, for a widget that reads it: the move and the spread since the last frame, both at each frame
    if (s.Gesture != ImGuiTouchPinchGesture_None && two_fingers && s.HasTaker && s.Taker.Data)
    {
        ImGuiTouchPinch& d = s.Data;
        if (started)
        {
            d = ImGuiTouchPinch();
            s.LastDistance = distance;
            s.LastCenter = center;
        }
        d.Active = true;
        d.Started = started;
        d.Ended = false;
        d.Center = center;
        d.CenterDelta = center - s.LastCenter;
        d.ScaleDelta = (s.LastDistance > 0.0f) ? distance / s.LastDistance : 1.0f;
        d.Scale = pinch_scale;
        d.Kind = (s.Gesture == ImGuiTouchPinchGesture_Pinch) ? ImGuiTouchPinchKind_Pinch
               : (s.Gesture == ImGuiTouchPinchGesture_Undecided) ? ImGuiTouchPinchKind_Undecided
                                                                  : ImGuiTouchPinchKind_Pan;
        s.LastDistance = distance;
        s.LastCenter = center;
    }

    if (s.Gesture != ImGuiTouchPinchGesture_None && !two_fingers)
        EndGesture(g, s);
}

}  // namespace ImGuiTouch
