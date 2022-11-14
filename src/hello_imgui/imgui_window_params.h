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
    If menu bar is shown, include or not the default app menu (with Quit button)

  * `showMenu_View`: _bool, default=true_.
    If menu bar is shown, include or not the default _View_ menu, that enables to change the layout and
    set the docked windows and status bar visibility)

  * `showStatusBar`: _bool, default=false_.
    Flag that enable to show a Status bar at the bottom. You can customize the status bar
    via RunnerCallbacks.ShowStatus()

  * `showStatus_Fps`: _bool, default=true_. If set, display the FPS in the status bar.

  * `configWindowsMoveFromTitleBarOnly`: _bool, default=true_.
    Make windows only movable from the title bar

  * `enableViewports`: _bool, default=false_. Enable multiple viewports (i.e multiple native windows)
    If true, you can drag windows outside out the main window in order to put their content into new native windows.

  * `tweakedTheme`: _string, default="ImGuiColorsDark"_.
    Change the ImGui theme. Several themes are available, you can query the list by calling
    HelloImGui::AvailableThemes()
@@md
 */
struct ImGuiWindowParams
{
    DefaultImGuiWindowType defaultImGuiWindowType = DefaultImGuiWindowType::ProvideFullScreenWindow;

    ImVec4 backgroundColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    bool showMenuBar = false;
    bool showMenu_App = true;
    bool showMenu_View = true;

    bool showStatusBar = false;
    bool showStatus_Fps = true;

    bool configWindowsMoveFromTitleBarOnly = true;

    bool enableViewports = false;

    ImGuiTheme::ImGuiTweakedTheme tweakedTheme;
};

}  // namespace HelloImGui