#pragma once

#include "hello_imgui/screen_bounds.h"
#include "imgui.h"
#include <vector>

namespace HelloImGui
{
namespace NullConfig
{
    ScreenBounds GetScreenBounds()
    {
        ScreenBounds bounds;
        bounds.size = {1920, 1080};
        bounds.position = {0, 0};
        return {bounds};
    }

    std::vector<ScreenBounds> GetMonitorsWorkAreas()
    {
        return {GetScreenBounds()};
    }

    float GetWindowSizeDpiScaleFactor() { return 1.f; }

}

}