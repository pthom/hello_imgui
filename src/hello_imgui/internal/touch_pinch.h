#pragma once
#include "hello_imgui/runner_params.h"

namespace HelloImGui
{
    // The platform reports the fingers on the screen before each frame (a platform that cannot tell reports nothing:
    // no pinch). pinchScale: the distance between the first two fingers, over their distance when the second landed.
    void SetTouchPointers(int count, float pinchScale, ImVec2 twoFingerMove = ImVec2(0.f, 0.f));

    // Applies the pinch, right after ImGui::NewFrame(): two fingers that spread or close scale the font
    // (style.FontScaleMain) while they stay on the screen; two fingers that move together are a right drag (the pan
    // of a node editor, the box of a plot). The swipe layer lets go of its press when the second finger lands.
    void UpdateTouchPinch(TouchPinchMode mode, bool interruptsWidgets);
}
