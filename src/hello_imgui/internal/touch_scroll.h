#pragma once
#include "hello_imgui/runner_params.h"

namespace HelloImGui
{
    // Scrolls the window under a finger that drags its content (the swipe), with inertia after the release.
    // Called by the runner at the end of each frame, after the user's GUI and before ImGui::Render().
    void UpdateTouchScroll(TouchScrollMode mode);
}
