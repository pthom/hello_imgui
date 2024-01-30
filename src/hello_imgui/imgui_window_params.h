#pragma once
#include "imgui.h"
#include <functional>
#include <string>
#include "hello_imgui/imgui_theme.h"
#include "hello_imgui/dpi_aware.h"


namespace HelloImGui
{
// @@md#DefaultImGuiWindowType

// `DefaultImGuiWindowType` is an enum class that defines whether a full screen background
// window is provided or not
enum class DefaultImGuiWindowType
{
    // `ProvideFullScreenWindow`: a full window is provided in the background
    ProvideFullScreenWindow,
    // `ProvideFullScreenDockSpace`: a full screen dockspace is provided in the background
    ProvideFullScreenDockSpace,
    // `NoDefaultWindow`: No default window is provided
    // (except for ImGui's default "debug" window)
    NoDefaultWindow
};
// @@md


// @@md#ImGuiWindowParams

// `ImGuiWindowParams` is a struct that defines the ImGui inner windows params
// These settings affect the imgui inner windows inside the application window.
// In order to change the application window settings, change the `AppWindowsParams`
struct ImGuiWindowParams
{
    // ------------ Main Options  -------------------------------------------------------

    // defaultImGuiWindowType: (enum DefaultImGuiWindowType)
    // Choose between:
    //    - ProvideFullScreenWindow (default)
    //      a full window is provided in the background
    //      You can still add windows on top of it, since the Z-order
    //      of this background window is always behind
    //    - ProvideFullScreenDockSpace:
    //      a full screen dockspace is provided in the background
    //      (use this if you intend to use docking)
    //    - NoDefaultWindow:
    //      no default window is provided
    DefaultImGuiWindowType defaultImGuiWindowType =
        DefaultImGuiWindowType::ProvideFullScreenWindow;

    // enableViewports: Enable multiple viewports (i.e. multiple native windows)
    // If true, you can drag windows outside the main window,
    // in order to put their content into new native windows.
    bool enableViewports = false;

    // Make windows only movable from the title bar
    bool configWindowsMoveFromTitleBarOnly = true;


    // ------------ Menus & Status bar --------------------------------------------------

    // Set the title of the App menu. If empty, the menu name will use
    // the "windowTitle" from AppWindowParams//
    std::string menuAppTitle = "";

    // Show Menu bar on top of imgui main window.
    // In order to fully customize the menu, set showMenuBar to true, and set showMenu_App
    // and showMenu_View params to false. Then, implement the callback
    // `RunnerParams.callbacks.ShowMenus`
    // which can optionally call `HelloImGui::ShowViewMenu` and `HelloImGui::ShowAppMenu`.
    bool showMenuBar = false;

    //  If menu bar is shown, include or not the default app menu
    bool showMenu_App = true;

    // Include or not a "Quit" item in the default app menu.
    // Set this to false if you intend to provide your own quit callback
    // with possible user confirmation
    // (and implement it inside RunnerCallbacks.ShowAppMenuItems)
    bool showMenu_App_Quit = true;

    // If menu bar is shown, include or not the default _View_ menu, that enables
    // to change the layout and set the docked windows and status bar visibility)
    bool showMenu_View = true;

    // Show theme selection in view menu
    bool showMenu_View_Themes = true;
    // `rememberTheme`: _bool, default=true_. Remember selected theme
    bool rememberTheme = true;

    // Flag that enable to show a Status bar at the bottom. You can customize
    // the status bar via RunnerCallbacks.ShowStatus()
    bool showStatusBar = false;

    // If set, display the FPS in the status bar.
    bool showStatus_Fps = true;
    // If set, showStatusBar and showStatus_Fps are stored in the application settings.
    bool rememberStatusBarSettings = true;


    // ------------ Change the dockspace or background window size -----------------------

    // If defaultImGuiWindowType = ProvideFullScreenWindow or ProvideFullScreenDockSpace,
    // you can set the position and size of the background window:
    //    - fullScreenWindow_MarginTopLeft is the window position
    //    - fullScreenWindow_MarginBottomRight is the margin between
    //      the "application window" bottom right corner
    //      and the "imgui background window" bottom right corner
    // Important note:
    //     In order to be Dpi aware, those sizes are in *em units*, not in pixels,
    //     i.e. in multiples of the font size! (See HelloImGui::EmToVec2)
    ImVec2 fullScreenWindow_MarginTopLeft     = ImVec2(0.f, 0.f);
    ImVec2 fullScreenWindow_MarginBottomRight = ImVec2(0.f, 0.f);


    // ------------ Theme ---------------------------------------------------------------

    // tweakedTheme: (enum ImGuiTheme::ImGuiTweakedTheme)
    // Changes the ImGui theme. Several themes are available, you can query the list
    // by calling HelloImGui::AvailableThemes()
    ImGuiTheme::ImGuiTweakedTheme tweakedTheme;

    // backgroundColor:
    // This is the "clearColor", i.e. the app window background color, is visible *only if*
    //    runnerParams.imGuiWindowParams.defaultImGuiWindowType = NoDefaultWindow
    // Alternatively, you can set your own RunnerCallbacks.CustomBackground to have full
    // control over what is drawn behind the Gui.
    ImVec4 backgroundColor = ImVec4(0.f, 0.f, 0.f, 0.f);

};
// @@md

}  // namespace HelloImGui