#pragma once
#include "hello_imgui/runner_params.h"

namespace HelloImGui
{
    // The swipe of a touch screen: a finger that drags the content of a window scrolls it, with inertia; a tap still
    // clicks, a short hold then a drag goes to the widget. Called by the runner right after ImGui::NewFrame(),
    // before any widget.
    void UpdateTouchScroll(TouchScrollMode mode, bool longPressIsRightClick);

    // A second finger landed (a pinch): the layer lets go of its press (no swipe, no tap, no inertia) and keeps the
    // widgets out of it until the fingers lift. When a widget holds the press (a hold happened), it is interrupted
    // only if asked. Returns false when a widget keeps its press.
    bool TouchScrollLetGo(bool evenAWidget);
}
