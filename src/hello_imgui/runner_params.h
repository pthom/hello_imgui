#pragma once
#include "hello_imgui/app_window_params.h"
#include "hello_imgui/imgui_window_params.h"
#include "hello_imgui/runner_callbacks.h"
#include "hello_imgui/docking_params.h"
#include "hello_imgui/backend_pointers.h"

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
    * `backendPointers`: _see [backend_pointers.h](backend_pointers.h)_.
       A struct that contains optional pointers to the backend implementations. These pointers will be filled
       when the application starts
    * `appShallExit`: _bool, default=false_.
       will be set to true by the app when exiting.
       _Note: 'appShallExit' has no effect on Mobile Devices (iOS, Android) and under emscripten, since these apps
       shall not exit._
    * `fps`: _int, default = 0` when applicable, set the application refresh rate
       (only used on emscripten for the moment: 0 stands for "let the app or the browser decide")

@@md
 */
struct RunnerParams
{
    RunnerCallbacks callbacks;
    AppWindowParams appWindowParams;
    ImGuiWindowParams imGuiWindowParams;
    DockingParams dockingParams;
    BackendPointers backendPointers;
    bool appShallExit = false;
    int fps = 0;
};

}  // namespace HelloImGui