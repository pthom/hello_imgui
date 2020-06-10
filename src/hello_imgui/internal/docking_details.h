#pragma once
#include "imgui.h"
#include "hello_imgui/docking_params.h"
#include "hello_imgui/imgui_window_params.h"
#include <functional>

namespace HelloImGui
{

// Internal functions below
namespace DockingDetails
{
void ConfigureImGuiDocking(const ImGuiWindowParams& imGuiWindowParams);
void ProvideWindowOrDock(const ImGuiWindowParams& imGuiWindowParams, DockingParams &dockingParams);
void CloseWindowOrDock(ImGuiWindowParams& imGuiWindowParams);
}  // namespace DockingDetails

}  // namespace HelloImGui