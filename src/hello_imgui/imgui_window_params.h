#pragma once
#include "imgui.h"
#include <functional>

namespace HelloImGui
{
enum class DefaultImGuiWindowType
{
    /// ProvideFullScreenWindow: a full window is provided in the background
    ProvideFullScreenWindow,
    /// ProvideFullScreenDockSpace: a full screen dockspace is provided in the background
    ProvideFullScreenDockSpace,
    /// No default window is provided (except for ImGui's default "debug" window)
    NoDefaultWindow
};

/// ImGuiWindowParams: defines the ImGui inner windows params
struct ImGuiWindowParams
{
    /// By default, a full window is provided in the background
    /// (you can still add window on top, since the Z-order of this background window is always behind)
    DefaultImGuiWindowType defaultImGuiWindowType = DefaultImGuiWindowType::ProvideFullScreenWindow;

    /// backgroundColor (only visible is defaultImGuiWindowType is NoDefaultWindow !)
    ImVec4 backgroundColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    /// Show Menu bar on top of imgui main window
    /// You can customize the menu via RunnerCallbacks.ShowMenus()
    bool showMenuBar = false;
    /// If menu bar is shown, include or not the default app menu (Quit + zoom) ?
    bool showMenu_App_QuitZoom = true;

    /// Show Status bar at the bottom ?
    /// You can customize the status bar via RunnerCallbacks.ShowStatus()
    bool showStatusBar = false;
    /// If the status bar is shown, display the FPS in it ?
    bool showStatus_Fps = true;

    /// Make windows only movable from the title bar
    bool configWindowsMoveFromTitleBarOnly = true;
};

}  // namespace HelloImGui