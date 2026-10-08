#pragma once
#include "hello_imgui/runner_params.h"

namespace HelloImGui
{
    // The swipe of a touch screen: a finger that drags the content of a window scrolls it, with inertia; a tap still
    // clicks, a short hold then a drag goes to the widget. On a touch screen, thinScrollbars makes the bars thin.
    // Called by the runner right after ImGui::NewFrame(), before any widget.
    void UpdateTouchScroll(TouchScrollMode mode, bool longPressIsRightClick, bool thinScrollbars);

    // The content dragged or flung past its end (the rubber band, the bounce) is drawn there: called by the runner
    // right after ImGui::Render(), before the draw data is rendered.
    void ApplyTouchOverscroll();

    // A second finger landed (a pinch): the layer lets go of its press (no swipe, no tap, no inertia) and keeps the
    // widgets out of it until the fingers lift. When a widget holds the press (a hold happened), it is interrupted
    // only if asked. Returns false when a widget keeps its press.
    bool TouchScrollLetGo(bool evenAWidget);

    // Two fingers that drag together (a right drag): the layer releases its press for good, nothing parked, so that
    // the widgets see the mouse. Returns false when a widget holds the press and must keep it.
    bool TouchScrollRelease(bool evenAWidget);
}
