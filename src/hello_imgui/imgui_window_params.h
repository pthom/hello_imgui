#pragma once
#include "imgui.h"
#include <functional>
#include <string>
#include "hello_imgui/imgui_theme.h"


namespace HelloImGui
{
/**
@@md#DefaultImGuiWindowType

__DefaultImGuiWindowType__ is an enum class that defines whether or not a full screen background window is provided.

 Values:
  * _ProvideFullScreenWindow_: a full window is provided in the background
  * _ProvideFullScreenDockSpace_: a full screen dockspace is provided in the background
  * _NoDefaultWindow_: No default window is provided (except for ImGui's default "debug" window)

@@md
 */
enum class DefaultImGuiWindowType
{
    ProvideFullScreenWindow,
    ProvideFullScreenDockSpace,
    NoDefaultWindow
};

/**
@@md#ImGuiWindowParams

__ImGuiWindowParams__ is a struct that defines the ImGui inner windows params
These settings affect the imgui inner windows inside the application window.
In order to change the application window settings, change the _AppWindowsParams_

 Members:

  * `defaultImGuiWindowType`: _DefaultImGuiWindowType, default=ProvideFullScreenWindow_.
      By default, a full window is provided in the background. You can still
      add windows on top of it, since the Z-order of this background window is always behind

  * `backgroundColor`: _ImVec4, default=ImVec4(0.45f, 0.55f, 0.60f, 1.00f)_.
      This is the "clearColor", only visible is defaultImGuiWindowType is NoDefaultWindow.

  * `showMenuBar`: _bool, default=false_.
    Show Menu bar on top of imgui main window
    You can customize the menu via `RunnerCallbacks.ShowMenus()`

  * `showMenu_App`: _bool, default=true_.
    If menu bar is shown, include or not the default app menu

   * `showMenu_App_Quit`: _bool, default=true_.
    Include or not a "Quit" item in the default app menu.
    Set this to false if you intend to provide your own quit callback with possible user confirmation
    (and implement it inside RunnerCallbacks.ShowAppMenuItems)

  * `showMenu_View`: _bool, default=true_.
    If menu bar is shown, include or not the default _View_ menu, that enables to change the layout and
    set the docked windows and status bar visibility)

  * `showStatusBar`: _bool, default=false_.
    Flag that enable to show a Status bar at the bottom. You can customize the status bar
    via RunnerCallbacks.ShowStatus()
  * `showStatus_Fps`: _bool, default=true_. If set, display the FPS in the status bar.
  * `rememberStatusBarSettings`: _bool, default=true_. If set, showStatusBar and showStatus_Fps are stored in the application settings.

  * `configWindowsMoveFromTitleBarOnly`: _bool, default=true_.
    Make windows only movable from the title bar

  * `enableViewports`: _bool, default=false_. Enable multiple viewports (i.e multiple native windows)
    If true, you can drag windows outside out the main window in order to put their content into new native windows.

   * `menuAppTitle`: _string, default=""_. Set the title of the App menu. If empty, this menu name will use
     the "windowTitle" from AppWindowParams

  * `tweakedTheme`: _ImGuiTheme::ImGuiTweakedTheme_.
    Change the ImGui theme. Several themes are available, you can query the list by calling
    HelloImGui::AvailableThemes()
  * `showMenu_View_Themes`: _bool, default=true_.
    Show theme selection in view menu
  * `rememberTheme`: _bool, default=true_.
    Remember selected theme
@@md
 */
struct ImGuiWindowParams
{
    DefaultImGuiWindowType defaultImGuiWindowType = DefaultImGuiWindowType::ProvideFullScreenWindow;

    ImVec4 backgroundColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    bool showMenuBar = false;
    bool showMenu_App = true;
    bool showMenu_App_Quit = true;
    bool showMenu_View = true;

    bool showStatusBar = false;
    bool showStatus_Fps = true;
    bool rememberStatusBarSettings = true;

    bool configWindowsMoveFromTitleBarOnly = true;

    bool enableViewports = false;

    std::string menuAppTitle = "";

    ImGuiTheme::ImGuiTweakedTheme tweakedTheme;
    bool showMenu_View_Themes = true;
    bool rememberTheme = true;
};

}  // namespace HelloImGui