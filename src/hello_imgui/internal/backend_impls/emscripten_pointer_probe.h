#pragma once
#ifdef __EMSCRIPTEN__
#include "imgui.h"
#include <string>

// The browser knows whether a pointer is a mouse, a finger or a pen (the pointerType of the pointer events), and
// GLFW has no way to tell ImGui. A probe on the document remembers the type of the last pointer event; the GLFW
// runner feeds it to ImGui (io.AddMouseSourceEvent) before each poll, so that ImGui's touch behaviours (the tooltips
// above the finger, the press trickling) and HelloImGui's swipe apply in the browser too.
namespace HelloImGui
{
    void InstallEmscriptenPointerProbe();          // once, after the page is loaded (a second call does nothing)
    void InstallEmscriptenViewportResizeRelay();   // the window's resize fired again when a page zoom changes the size
    ImGuiMouseSource LastEmscriptenPointerSource();  // the type of the last pointer event (Mouse before any)
    bool EmscriptenPointerIsDown();                  // a finger or a button is down (pointerdown, no pointerup yet)
    int EmscriptenFingerCount();                     // the fingers on the screen
    // A tap (a finger that lands alone, and lifts within half a second) that starts and ends in one of these
    // rectangles (ImGui's screen coordinates) opens its url in a new tab, from inside the browser's touch handler (the
    // only place a new tab is allowed).
    void EmscriptenRequestTapZone(ImVec2 rectMin, ImVec2 rectMax, const std::string& url);  // for this frame
    void EmscriptenPushTapZones(float displayWidth);  // before each poll: the last frame's zones (pushed when they change)
    float EmscriptenPinchScale();                    // the distance between the first two fingers, over the one when the second landed
    ImVec2 EmscriptenTwoFingerMove();                // how far the two fingers' middle moved since the second landed (ImGui's units)
    bool EmscriptenHasTouchScreen();                 // navigator.maxTouchPoints > 0: a hint for the whole session
}
#endif
