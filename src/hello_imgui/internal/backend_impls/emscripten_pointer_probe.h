#pragma once
#ifdef __EMSCRIPTEN__
#include "imgui.h"
#include <string>

// What a browser adds to HelloImGui's runners (the touch input itself: ImGuiTouch::SetupEmscripten(), in imgui_kit)
namespace HelloImGui
{
    void InstallEmscriptenTapZones();              // once, after the page is loaded (a second call does nothing)
    void InstallEmscriptenViewportResizeRelay();   // the window's resize fired again when a page zoom changes the size
    // A tap (a finger that lands alone, and lifts within half a second) that starts and ends in one of these
    // rectangles (ImGui's screen coordinates) opens its url in a new tab, from inside the browser's touch handler (the
    // only place a new tab is allowed).
    void EmscriptenRequestTapZone(ImVec2 rectMin, ImVec2 rectMax, const std::string& url);  // for this frame
    void EmscriptenPushTapZones(float displayWidth);  // before each poll: the last frame's zones (pushed when they change)
}
#endif
