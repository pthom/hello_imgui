#pragma once
#include "hello_imgui/app_window_params.h"
#include "hello_imgui/imgui_window_params.h"
#include "hello_imgui/runner_callbacks.h"
#include "hello_imgui/docking_params.h"
#include "hello_imgui/backend_pointers.h"

#include <vector>

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
 @@md#FpsIdling

**FpsIdling** is a struct that contains Fps Idling parameters

* `fpsIdle`: _float, default=9_.
  ImGui applications can consume a lot of CPU, since they update the screen very frequently.
  In order to reduce the CPU usage, the FPS is reduced when no user interaction is detected.
  This is ok most of the time but if you are displaying animated widgets (for example a live video),
  you may want to ask for a faster refresh: either increase fpsIdle, or set it to 0 for maximum refresh speed
  (you can change this value during the execution depending on your application refresh needs)
* `enableIdling`: _bool, default=true_.
  Set this to false to disable idling (this can be changed dynamically during execution)
* `isIdling`: bool (dynamically updated during execution)
  This bool will be updated during the application execution, and will be set to true when it is idling.
* `rememberEnableIdling`: _bool, default=true_.
  If true, the last value of enableIdling is restored from the settings at startup.
@@md
*/
struct FpsIdling
{
    float fpsIdle = 9.f;
    bool  enableIdling = true;
    bool  isIdling = false;
    bool  rememberEnableIdling = true;
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
* `alternativeDockingLayouts`: _vector<DockingParams>, default=empty_
   List of possible additional layout for the applications. Only used in advanced cases when several layouts are available.
* `rememberSelectedAlternativeLayout`: _bool, default=true_
   Shall the application remember the last selected layout. Only used in advanced cases when several layouts are available.
* `backendPointers`: _see [backend_pointers.h](backend_pointers.h)_.
   A struct that contains optional pointers to the backend implementations. These pointers will be filled
   when the application starts
* `backendType`: _enum BackendType, default=BackendType::FirstAvailable_
  Select the wanted backend type between `Sdl`, `Glfw` and `Qt`. Only useful when multiple backend are compiled
  and available.
* `fpsIdling`: _FpsIdling_. Idling parameters (set fpsIdling.enableIdling to false to disable Idling)
* `iniFilename`: _string, default = ""_
  Sets the ini filename under which imgui will save its params. Path is relative to the current app working dir.
  If empty, then the ini file name will be derived from appWindowParams.windowTitle (if both are empty, the ini filename will be imgui.ini).
  Note: if appWindowParams.restorePreviousGeometry is true, then HelloImGui will also store the app window size and position into "iniFilename + _appWindow.ini"
* `iniFilename_useAppWindowTitle`: _bool, default = true_.
  Shall the iniFilename be derived from appWindowParams.windowTitle if empty
* `appShallExit`: _bool, default=false_.
   During execution, set this to true to exit the app.
   _Note: 'appShallExit' has no effect on Mobile Devices (iOS, Android) and under emscripten, since these apps
   shall not exit._
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
    std::vector<DockingParams> alternativeDockingLayouts;
    bool rememberSelectedAlternativeLayout = true;

    BackendPointers backendPointers;
    BackendType backendType = BackendType::FirstAvailable;
    FpsIdling fpsIdling;

    std::string iniFilename = "";
    bool iniFilename_useAppWindowTitle = true;

    bool appShallExit = false;
    int emscripten_fps = 0;
};


/**
 @@md#SimpleRunnerParams

**SimpleRunnerParams** is a struct that contains simpler params adapted for simple use cases.

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
* `fpsIdle`: _float, default=9_.
   FPS of the application when idle (set to 0 for full speed).

For example, this is sufficient to run an application:

```cpp
void MyGui() {
    ImGui::Text("Hello, world");
    if (ImGui::Button("Exit"))
        HelloImGui::GetRunnerParams()->appShallExit = true;
}

int main(){
    auto params = HelloImGui::SimpleRunnerParams {.guiFunction = MyGui, .windowSizeAuto = true, .windowTitle = "Example"};
    HelloImGui::Run(params);
}
```

@@md
 */
struct SimpleRunnerParams
{
    VoidFunction guiFunction = EmptyVoidFunction();
    std::string windowTitle = "";

    bool windowSizeAuto = false;
    bool windowRestorePreviousGeometry = false;
    ScreenSize windowSize = DefaultWindowSize;

    float fpsIdle = 9.f;
    bool  enableIdling = true;

    RunnerParams ToRunnerParams() const;
};

}  // namespace HelloImGui