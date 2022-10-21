#pragma once
#include "imgui.h"
#include "hello_imgui/runner_callbacks.h"
#include <functional>
#include <string>
#include <vector>
#include <utility>
#include <stdio.h>

namespace HelloImGui
{
/**
@@md#DockingIntro

HelloImGui facilitates the use of dockable windows (based on ImGui [docking branch](https://github.com/ocornut/imgui/tree/docking)).

You can easily specify the default layout of the dockable windows, as well as their GUI code.
HelloImGui will then provide a "View" menu with options in order to show/hide the dockable windows, and to restore the default layout

![demo docking](../../docs/images/docking.gif)

Source for this example: [src/hello_imgui_demos/hello_imgui_demodocking](../../src/hello_imgui_demos/hello_imgui_demodocking)

This is done via the `DockingParams` struct: its member `dockingSplits` specifies the layout,
and its member `dockableWindows` specifies the list of dockable windows, along with their default location,
and their code (given by lambdas). See doc below for more details.

 @@md


@@md#DockingExample

Docking params: Example usage

````cpp
HelloImGui::RunnerParams runnerParams;
runnerParams.imGuiWindowParams.defaultImGuiWindowType =
    HelloImGui::DefaultImGuiWindowType::ProvideFullScreenDockSpace;

runnerParams.dockingParams.dockingSplits =
{
    // First, add a bottom space whose height is 25% of the app height
    // This will split the preexisting default dockspace "MainDockSpace"
    // in two parts.
    { "MainDockSpace", "BottomSpace", ImGuiDir_Down, 0.25 },
    // Then, add a space to the left which occupies a column
    // whose width is 25% of the app height
    { "MainDockSpace", "LeftSpace", ImGuiDir_Left, 0.25 }
    // We now have three spaces: "MainDockSpace", "BottomSpace", and "LeftSpace"
};
runnerParams.dockingParams.dockableWindows =
{
    // A Window named "Main" will be placed in "MainDockSpace".
    // Its Gui is provided by the VoidFunction "MainGui"
    {"Main", "MainDockSpace", MainGui},
    // A Log  window named "Logs" will be placed in "BottomSpace".
    // Its Gui is provided by the VoidFunction "ShowLogs"
    {"Logs", "BottomSpace", ShowLogs},
    // A Command panel named "Commands" will be placed in "LeftSpace".
    // Its Gui is provided by the VoidFunction "ShowCommandsPanel"
    {"Commands", "LeftSpace", ShowCommandsPanel}
};

runnerParams.imGuiWindowParams.showMenuBar = true;
runnerParams.imGuiWindowParams.showStatusBar = true;

HelloImGui::Run(runnerParams);
````

@@md
*/

/*****************************************************************************/

// A DockSpaceName is a simple string that identifies a zone on the screen where windows can be docked.
using DockSpaceName = std::string;

/**
@@md#DockingSplit

**DockingSplit** is a struct that defines the way the docking splits should be applied on the screen
in order to create new Dock Spaces. _DockingParams_ contains a _vector[DockingSplit]_,
in order to partition the screen at your will.

_Members:_

* `initialDock`: _DockSpaceName (aka string)_

    id of the space that should be split.
    At the start, there is only one Dock Space named "MainDockSpace".
    You should start by partitioning this space, in order to create a new dock space.

* `newDock`: _DockSpaceName (aka string)_. id of the new dock space that will be created
* `direction`: *ImGuiDir_ (enum with ImGuiDir_Down, ImGuiDir_Down, ImGuiDir_Left, ImGuiDir_Right)*.
Direction where this dock space should be created
* `ratio`: _float, default=0.25f_. Ratio of the initialDock size that should be used by the new dock space

@@md
*/
struct DockingSplit
{
    DockingSplit(const DockSpaceName& initialDock_ = {}, const DockSpaceName& newDock_ = {}, 
                 ImGuiDir_ direction_ = ImGuiDir_Down, float ratio_ = 0.25f) 
      : initialDock(initialDock_), newDock(newDock_), direction(direction_), ratio(ratio_) {}

    DockSpaceName initialDock;
    DockSpaceName newDock;
    ImGuiDir_ direction;
    float ratio = 0.25f;
};

/**
@@md#DockableWindow

**DockableWindow** is a struct that represents a window that can be docked.

_Members:_

* `label`: _string_. Title of the window.
* `dockSpaceName`: _DockSpaceName (aka string)_. Id of the dock space where this window
   should initialy be placed
* `GuiFunction`: _VoidFuntion_. Any function that will render this window's Gui.
* `isVisible`: _bool, default=true_. Flag that indicates whether this window is visible or not.
* `canBeClosed`: _bool, default=true_. Flag that indicates whether the user can close this window.
* `callBeginEnd`: _bool, default=true_. Flag that indicates whether ImGui::Begin and ImGui::End
   calls should be added automatically (with the given "label"). Set to false if you want to call
   ImGui::Begin/End yourself
* `includeInViewMenu`: _bool, default=true_. Flag that indicates whether this window should be mentioned
   in the view menu.
* `imGuiWindowFlags`: _ImGuiWindowFlags, default=0_. Window flags, see enum ImGuiWindowFlags_
* `windowSize`: _ImVec2, default=(0.f, 0.f) (i.e let the app decide)_. Window size (unused if docked)
* `windowSizeCondition`: _ImGuiCond, default=ImGuiCond_FirstUseEver_. When to apply the window size.
* `windowPos`: _ImVec2, default=(0.f, 0.f) (i.e let the app decide)_. Window position (unused if docked)
* `windowPosCondition`: _ImGuiCond, default=ImGuiCond_FirstUseEver_. When to apply the window position.
* `focusWindowAtNextFrame`: _bool, default = false_. If set to true this window will be focused at the next frame.

@@md
**/
struct DockableWindow
{
    DockableWindow(
        const std::string & label_ = "",
        const DockSpaceName & dockSpaceName_ = "",
        const VoidFunction guiFonction_ = {},
        bool isVisible_ = true,
        bool canBeClosed_ = true)
    : label(label_), dockSpaceName(dockSpaceName_),
      GuiFunction(guiFonction_),
      isVisible(isVisible_),
      canBeClosed(canBeClosed_) {}

    std::string label;

    DockSpaceName dockSpaceName;

    VoidFunction GuiFunction = {};

    bool isVisible = true;
    bool canBeClosed = true;
    bool callBeginEnd = true;
    bool includeInViewMenu = true;
    ImGuiWindowFlags imGuiWindowFlags = 0;

    ImVec2 windowSize = ImVec2(0.f, 0.f);
    ImGuiCond  windowSizeCondition = ImGuiCond_FirstUseEver;

    ImVec2 windowPosition = ImVec2(0.f, 0.f);
    ImGuiCond  windowPositionCondition = ImGuiCond_FirstUseEver;

    bool focusWindowAtNextFrame = false;
};

/**
@@md#DockingParams

**DockingParams** contains all the settings concerning the docking,
 together _with the Gui functions for the docked windows_.

 _Members:_

* `dockingSplits`: _vector[DockingSplit]_.
  Defines the way docking splits should be applied on the screen in order to create new Dock Spaces
* `dockableWindows`: _vector[DockableWindow]_.
  List of the dockable windows, together with their Gui code
* `resetUserDockLayout`: _bool, default=true_.
  Reset user layout at application startup

 _Helpers:_

 * `DockableWindow * dockableWindowOfName(const std::string & name)`: returns a pointer to a dockable window
 * `void focusDockableWindow(const std::string& name)`: will focus a dockable window

@@md
 */
struct DockingParams
{
    std::vector<DockingSplit> dockingSplits;

    std::vector<DockableWindow> dockableWindows;

    bool resetUserDockLayout = true;

    // wasDockLayoutApplied is an internal variable
    bool wasDockLayoutApplied = false;

    DockableWindow * dockableWindowOfName(const std::string & name);
    void focusDockableWindow(const std::string& windowName);
};
} // namespace HelloImGui
