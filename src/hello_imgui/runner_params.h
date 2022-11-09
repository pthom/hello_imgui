#pragma once
#include "hello_imgui/app_window_params.h"
#include "hello_imgui/imgui_window_params.h"
#include "hello_imgui/runner_callbacks.h"
#include "hello_imgui/docking_params.h"
#include "hello_imgui/backend_pointers.h"


namespace HelloImGui
{

enum class BackendType
{
    FirstAvailable,
    Sdl,
    Glfw,
    Qt
};

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
* `backendType`: _enum BackendType, default=BackendType::FirstAvailable_
  Select the wanted backend type between `Sdl`, `Glfw` and `Qt`. Only useful when multiple backend are compiled
  and available.
* `appShallExit`: _bool, default=false_.
   Will be set to true by the app when exiting.
   _Note: 'appShallExit' has no effect on Mobile Devices (iOS, Android) and under emscripten, since these apps
   shall not exit._
* `fpsIdle`: _float, default=10_.
  ImGui applications can consume a lot of CPU, since they update the screen very frequently.
  In order to reduce the CPU usage, the FPS is reduced when no user interaction is detected.
  This is ok most of the time but if you are displaying animated widgets (for example a live video),
  you may want to ask for a faster refresh: either increase fpsIdle, or set it to 0 for maximum refresh speed
  (you can change this value during the execution depending on your application refresh needs)
* `emscripten_fps`: _int, default = 0_.
  Set the application refresh rate (only used on emscripten: 0 stands for "let the app or the browser decide")
@@md
 */
struct RunnerParams
{
    RunnerCallbacks callbacks;
    AppWindowParams appWindowParams;
    ImGuiWindowParams imGuiWindowParams;
    DockingParams dockingParams;
    BackendPointers backendPointers;
    BackendType backendType = BackendType::FirstAvailable;
    bool appShallExit = false;

    float fpsIdle = 10.f;

    int emscripten_fps = 0;
};


/**
 @@md#SimpleRunnerParams

**RunnerParams** is a struct that contains simpler params adapted for simple uses

 Members:
* `guiFunction`: _VoidFunction_.
   Function that renders the Gui.
* `windowTitle`: _string, default=""_.
   Title of the application window
* `windowSizeAuto`: _bool, default=false_.
   If true, the size of the window will be computed from its widgets.
* `windowRestorePreviousGeometry`: _bool, default=true_.
   If true, restore the size and position of the window between runs.
* `windowSize`: _ScreenSize, default={800, 600}_.
   Size of the window
* `fpsIdle`: _float, default=10_.
   FPS of the application when idle (set to 0 for full speed).
@@md
 */
struct SimpleRunnerParams
{
    VoidFunction guiFunction;
    std::string windowTitle = "";

    bool windowSizeAuto = false;
    bool windowRestorePreviousGeometry = false;
    ScreenSize windowSize = {800, 600};

    float fpsIdle = 10.f;

    RunnerParams ToRunnerParams() const;
};

}  // namespace HelloImGui