#pragma once
#include "imgui.h"
#include <functional>

namespace HelloImGui
{

/// ImGuiWindowParams: defines the ImGui inner windows params
struct ImGuiWindowParams
{
    enum class ImGuiDefaultWindowType
    {
        ProvideFullScreenWindow,
        ProvideFullScreenDockSpace,
        NoDefaultWindow
    };

    ImVec4 BackgroundColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImGuiDefaultWindowType DefaultWindowType = ImGuiDefaultWindowType::ProvideFullScreenWindow;
    bool ShowMenuBar = false;
    bool ConfigWindowsMoveFromTitleBarOnly = true;

    std::function<void(ImGuiID /* fullDockSpace */)> InitialDockLayoutFunction = nullptr;
    bool WasDockLayoutApplied = false;
};

// Internal functions below
namespace ImGuiWindowParamsFunctions
{
    void ConfigureImGuiDocking(const ImGuiWindowParams& imGuiWindowParams);
    void ProvideWindowOrDock(ImGuiWindowParams& imGuiWindowParams);
    void CloseWindowOrDock(ImGuiWindowParams& imGuiWindowParams);
    void ResetDockLayout(ImGuiWindowParams& imGuiWindowParams);
}  // namespace ImGuiWindowParamsFunctions

}  // namespace HelloImGui