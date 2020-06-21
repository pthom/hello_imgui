#pragma once
#include "hello_imgui/app_window_params.h"
#include "hello_imgui/imgui_window_params.h"
#include "hello_imgui/runner_callbacks.h"
#include "hello_imgui/docking_params.h"

namespace HelloImGui
{

/**
 @@md#RunnerParams
 **RunnerParams** is a struct that contains all the settings and callbacks needed to run an application.

 Members:
* `callbacks`: _see [runner_callbacks.h](runner_callbacks.h)_.
    callbacks.ShowGui() will render the gui, ShowMenus() will show the menus, etc.
* `appWindowParams`: _see [app_window_params.h](app_window_params.h)_.
    application Window Params (position, size, title)
* `imGuiWindowParams`: _see [imgui_window_params.h](imgui_window_params.h)_.
    imgui window params (use docking, showMenuBar, ProvideFullScreenWindow, etc)
* `dockingParams`: _see [docking_params.h](docking_params.h)_.
    dockable windows content and layout
* `appShallExit`: _bool, default=false_.
   will be set to true by the app when exiting.

   _Note: 'appShallExit' has no effect on Mobile Devices (iOS, Android) and under emscripten, since these apps
   shall not exit._
@@md
 */
struct RunnerParams
{
    RunnerCallbacks callbacks;
    AppWindowParams appWindowParams;
    ImGuiWindowParams imGuiWindowParams;
    DockingParams dockingParams;
    bool appShallExit = false;
};

}  // namespace HelloImGui