#pragma once
#include "hello_imgui/runner_params.h"

namespace HelloImGui
{
    // The swipe of a touch screen: a finger that drags the content of a window scrolls it, with inertia; a tap still
    // clicks, a short hold then a drag goes to the widget. Called by the runner right after ImGui::NewFrame(),
    // before any widget.
    void UpdateTouchScroll(TouchScrollMode mode);
}
