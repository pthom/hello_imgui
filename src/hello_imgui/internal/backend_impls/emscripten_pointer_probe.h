#pragma once
#ifdef __EMSCRIPTEN__
#include "imgui.h"

// The browser knows whether a pointer is a mouse, a finger or a pen (the pointerType of the pointer events), and
// GLFW has no way to tell ImGui. A probe on the document remembers the type of the last pointer event; the GLFW
// runner feeds it to ImGui (io.AddMouseSourceEvent) before each poll, so that ImGui's touch behaviours (the tooltips
// above the finger, the press trickling) and HelloImGui's swipe apply in the browser too.
namespace HelloImGui
{
    void InstallEmscriptenPointerProbe();          // once, after the page is loaded (a second call does nothing)
    ImGuiMouseSource LastEmscriptenPointerSource();  // the type of the last pointer event (Mouse before any)
    bool EmscriptenPointerIsDown();                  // a finger or a button is down (pointerdown, no pointerup yet)
    int EmscriptenFingerCount();                     // the fingers on the screen
    float EmscriptenPinchScale();                    // the distance between the first two fingers, over the one when the second landed
    bool EmscriptenHasTouchScreen();                 // navigator.maxTouchPoints > 0: a hint for the whole session
}
#endif
