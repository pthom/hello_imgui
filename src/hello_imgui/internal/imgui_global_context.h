#pragma once
#include "imgui.h"

#ifdef HELLO_IMGUI_IMGUI_SHARED
extern IMGUI_API ImGuiContext* GImGui;  // ImGui Global Context needs to be redefined accross DLL boundaries
#endif
