// imgui_kit: what its sources share, not part of its API
// https://github.com/pthom/imgui_kit (MIT license)
#pragma once
#include "imgui_kit.h"

namespace ImGuiKit
{
namespace Internal
{
    // True between FillRowTail() and EndFillRow(): the fill row makes the room, a widget must not wrap
    bool IsInFillRowTail();
}
}

// The touch layer's steps, which ImGuiTouch::Setup() calls through Dear ImGui's context hooks (the tests call them
// directly, frame by frame)
namespace ImGuiTouch
{
    // The swipe: a finger that drags the content of a window scrolls it, with inertia; a tap still clicks, a short hold
    // then a drag goes to the widget. On a touch screen, thinScrollbars makes the bars thin.
    // Right after ImGui::NewFrame(), before any widget.
    void UpdateTouchScroll(ImGuiTouchScrollMode mode, bool longPressIsRightClick, bool thinScrollbars);

    // A thin scroll bar held by a finger widens over the content (iOS): after the GUI, when this frame's scroll is final
    void DrawTouchScrollbarGrab();

    // The content dragged or flung past its end (the rubber band, the bounce): right after ImGui::Render(), before the
    // draw data is rendered
    void ApplyTouchOverscroll();

    // A second finger landed (a pinch): the layer lets go of its press (no swipe, no tap, no inertia) and keeps the
    // widgets out of it until the fingers lift. When a widget holds the press (a hold happened), it is interrupted
    // only if asked. Returns false when a widget keeps its press.
    bool TouchScrollLetGo(bool evenAWidget);

    // Two fingers that drag together (a right drag): the layer releases its press for good, nothing parked, so that
    // the widgets see the mouse. Returns false when a widget holds the press and must keep it.
    bool TouchScrollRelease(bool evenAWidget);

    // The pinch, right after ImGui::NewFrame(): two fingers that spread or close scale the font (style.FontScaleMain)
    // while they stay on the screen; two fingers that move together are a right drag. The swipe lets go of its press
    // when the second finger lands.
    void UpdateTouchPinch(ImGuiTouchPinchMode mode, bool interruptsWidgets);

namespace Internal
{
    // The content moves on its own this frame (the inertia, the bounce, an animation): IsAnimating() is true
    void RequestAnimation();
    // For the tests, which call the steps without the hooks: whether an animation was asked since the last call
    bool ConsumeAnimationRequest();
}
}

namespace ImGuiKit
{
    // The wheel session's two steps: right before ImGui::NewFrame() (the item that claimed the wheel in the last frame,
    // before NewFrame() gives it back to a window), and right after it, before any widget
    void WheelSessionBeforeNewFrame();
    void UpdateWheelSession(bool enabled);
}
