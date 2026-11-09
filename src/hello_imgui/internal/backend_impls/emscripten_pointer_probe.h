#pragma once
#ifdef __EMSCRIPTEN__
#include "imgui.h"
#include <string>

// What a browser adds to HelloImGui's runners (the touch input itself: ImGuiTouch::SetupEmscripten(), in imgui_kit)
namespace HelloImGui
{
    void InstallEmscriptenViewportResizeRelay();   // the window's resize fired again when a page zoom changes the size
}
#endif
