#pragma once

#if defined(__ANDROID__) && defined(HELLOIMGUI_USE_SDL2)
// We need to include SDL, so that it can instantiate its main function under Android
#include "SDL.h"
#endif

#include "hello_imgui/dpi_aware.h"
#include "hello_imgui/hello_imgui_assets.h"
#include "hello_imgui/hello_imgui_error.h"
#include "hello_imgui/hello_imgui_logger.h"
#include "hello_imgui/image_from_asset.h"
#include "hello_imgui/imgui_theme.h"
#include "hello_imgui/hello_imgui_theme.h"
#include "hello_imgui/hello_imgui_font.h"
#include "hello_imgui/runner_params.h"
#include "hello_imgui/hello_imgui_widgets.h"
#include <string>

#include <cstddef>
#include <cstdint>

#ifdef HELLOIMGUI_USE_SDL2
    #ifdef _WIN32
        #ifndef HELLOIMGUI_WIN32_AUTO_WINMAIN
            // Under Windows, we redefine WinMain ourselves
            #include <SDL.h>
        #endif
    #endif
     // Let SDL redefine main under iOS
    #ifdef HELLOIMGUI_IOS
        #include <SDL.h>
    #endif
#endif

struct ImGuiTestEngine;


namespace HelloImGui
{

// =========================== HelloImGui::Run ==================================
/**
@@md#HelloImGui::Run

__HelloImGui::Run()__ will run an application with a single call.

Three signatures are provided:

* `HelloImGui::Run(RunnerParams &)`: full signature, the most customizable version.
  Runs an application whose params and Gui are provided by runnerParams.

* `HelloImGui::Run(const SimpleRunnerParams&)`:
  Runs an application, using simpler params.

* `HelloImGui::Run(guiFunction, windowTitle, windowSize, windowSizeAuto=false, restoreLastWindowGeometry=false, fpsIdle=10)`
  Runs an application, by providing the Gui function, the window title, etc.

Although the API is extremely simple, it is highly customizable, and you can set many options by filling
the elements in the `RunnerParams` struct, or in the simpler  `SimpleRunnerParams`.

__HelloImGui::GetRunnerParams()__  will return the runnerParams of the current application.

@@md
*/

// `HelloImGui::Run(RunnerParams &)`: full signature, the most customizable version.
// Runs an application whose params and Gui are provided by runnerParams.
void Run(RunnerParams &runnerParams);

// `HelloImGui::Run(const SimpleRunnerParams&)`:
// Runs an application, using simpler params.
void Run(const SimpleRunnerParams &simpleParams);

// Runs an application, by providing the Gui function, the window title, etc.
void Run(
    const VoidFunction &guiFunction,
    const std::string &windowTitle = "",
    bool windowSizeAuto = false,
    bool windowRestorePreviousGeometry = false,
    const ScreenSize &windowSize = DefaultWindowSize,
    float fpsIdle = 10.f
);

// =========================== HelloImGui::ManualRender ==================================
// @@md#HelloImGui::ManualRender

namespace ManualRender
{
    // HelloImGui::ManualRender is a namespace that groups functions, allowing fine-grained control over the rendering process:
    // - It is customizable like HelloImGui::Run: initialize it with `RunnerParams` or `SimpleRunnerParams`
    // - `ManualRender::Render()` will render the application for one frame:
    // - Ensure that `ManualRender::Render()` is triggered regularly (e.g., through a loop or other mechanism)
    //   to maintain responsiveness. This method must be called on the main thread.
    //
    // A typical use case is:
    // C++
    //        ```cpp
    //        HelloImGui::RunnerParams runnerParams;
    //        runnerParams.callbacks.ShowGui = ...; // your GUI function
    //        // Optionally, choose between Sleep, EarlyReturn, or Auto for fps idling mode:
    //        // runnerParams.fpsIdling.fpsIdlingMode = HelloImGui::FpsIdlingMode::Sleep; // or EarlyReturn, Auto
    //        HelloImGui::ManualRender::SetupFromRunnerParams(runnerParams);
    //        while (!HelloImGui::GetRunnerParams()->appShallExit)
    //        {
    //            HelloImGui::ManualRender::Render();
    //        }
    //        HelloImGui::ManualRender::TearDown();
    //        ```
    // Python:
    //        ```python
    //        runnerParams = HelloImGui.RunnerParams()
    //        runnerParams.callbacks.show_gui = ... # your GUI function
    //        while not hello_imgui.get_runner_params().app_shall_exit:
    //            hello_imgui.manual_render.render()
    //        hello_imgui.manual_render.tear_down()
    //        ```
    //
    // **Notes:**
    //  1. Depending on the configuration (`runnerParams.fpsIdling.fpsIdlingMode`), `HelloImGui` may enter
    //     an idle state to reduce CPU usage, if no events are received (e.g., no input or interaction).
    //     In this case, `Render()` will either sleep or return immediately.
    //     By default,
    //       - On Emscripten, `ManualRender::Render()` will return immediately to avoid blocking the main thread.
    //       - On other platforms, it will sleep
    //  2. If initialized with `RunnerParams`, a copy of the `RunnerParams` will be made
    //     (which can be accessed with `HelloImGui::GetRunnerParams()`).

    // Initializes the rendering with the full customizable `RunnerParams`.
    // This will initialize the platform backend (SDL, Glfw, etc.) and the rendering backend (OpenGL, Vulkan, etc.).
    // A distinct copy of `RunnerParams` is stored internally.
    void SetupFromRunnerParams(const RunnerParams& runnerParams);

    // Initializes the rendering with `SimpleRunnerParams`.
    // This will initialize the platform backend (SDL, Glfw, etc.) and the rendering backend (OpenGL, Vulkan, etc.).
    void SetupFromSimpleRunnerParams(const SimpleRunnerParams& simpleParams);

    // Initializes the renderer with a simple GUI function and additional parameters.
    // This will initialize the platform backend (SDL, Glfw, etc.) and the rendering backend (OpenGL, Vulkan, etc.).
    void SetupFromGuiFunction(
        const VoidFunction& guiFunction,
        const std::string& windowTitle = "",
        bool windowSizeAuto = false,
        bool windowRestorePreviousGeometry = false,
        const ScreenSize& windowSize = DefaultWindowSize,
        float fpsIdle = 10.f
    );

    // Renders the current frame. Should be called regularly to maintain the application's responsiveness.
    void Render();

    // Tears down the renderer and releases all associated resources.
    // This will release the platform backend (SDL, Glfw, etc.) and the rendering backend (OpenGL, Vulkan, etc.).
    // After calling `TearDown()`, the InitFromXXX can be called with new parameters.
    void TearDown();
} // namespace ManualRender

// @@md


// ============================== Utility functions ===============================

// @@md#UtilityFunctions

// `GetRunnerParams()`:  a convenience function that will return the runnerParams
// of the current application
    RunnerParams* GetRunnerParams();

// `IsUsingHelloImGui()`: returns true if the application is using HelloImGui
    bool IsUsingHelloImGui();

// `FrameRate(durationForMean = 0.5)`: Returns the current FrameRate.
//  May differ from ImGui::GetIO().FrameRate, since one can choose the duration
//  for the calculation of the mean value of the fps
//  Returns the current FrameRate. May differ from ImGui::GetIO().FrameRate,
//  since one can choose the duration for the calculation of the mean value of the fps
//  (Will only lead to accurate values if you call it at each frame)
float FrameRate(float durationForMean = 0.5f);

// `ImGuiTestEngine* GetImGuiTestEngine()`: returns a pointer to the global instance
//  of ImGuiTestEngine that was initialized by HelloImGui
//  (iif ImGui Test Engine is active).
ImGuiTestEngine* GetImGuiTestEngine();

// `GetBackendDescription()`: returns a string with the backend info
// Could be for example:
//     "Glfw - OpenGL3"
//     "Glfw - Metal"
//     "Sdl - Vulkan"
std::string GetBackendDescription();

// `ChangeWindowSize(const ScreenSize &windowSize)`: sets the window size
// (useful if you want to change the window size during execution)
void ChangeWindowSize(const ScreenSize &windowSize);


// `UseWindowFullMonitorWorkArea()`: sets the window size to the monitor work area
// (useful if you want to change the window size during execution)
void UseWindowFullMonitorWorkArea();

// @@md


// ============================== Layout Utils =============================

// @@md#HelloImGui::Layouts

// In advanced cases when several layouts are available, you can switch between layouts.
// See demo inside
//     https://github.com/pthom/hello_imgui/tree/master/src/hello_imgui_demos/hello_imgui_demodocking/hello_imgui_demodocking.main.cpp

// `SwitchLayout(layoutName)`
//  Changes the application current layout. Only used in advanced cases
//  when several layouts are available, i.e. if you filled
//      runnerParams.alternativeDockingLayouts.
void           SwitchLayout(const std::string& layoutName);

// `CurrentLayoutName()`: returns the name of the current layout
std::string    CurrentLayoutName();

// `AddDockableWindow()`: will add a dockable window to the current layout.
// Will dock the window to the dockspace it belongs to if forceDockspace is true,
// otherwise will dock it to the last space it was docked to (using saved settings)
void AddDockableWindow(const DockableWindow& dockableWindow, bool forceDockspace = false);

// `RemoveDockableWindow()`: will remove a dockable window from the current layout.
// (dockableWindowName is the label of the window, as provided in the DockableWindow struct)
void RemoveDockableWindow(const std::string& dockableWindowName);

// @@md


// ============================== User prefs Utils =============================

// @@md#HelloImGui::UserPref

// You may store additional user settings in the application settings.
// This is provided as a convenience only, and it is not intended to store large
// quantities of text data. Use sparingly.

// `SaveUserPref(string userPrefName, string userPrefContent)`:
//  Shall be called in the callback runnerParams.callbacks.BeforeExit
void        SaveUserPref(const std::string& userPrefName, const std::string& userPrefContent);

// `string LoadUserPref(string& userPrefName)`
//  Shall be called in the callback runnerParams.callbacks.PostInit
std::string LoadUserPref(const std::string& userPrefName);
// @@md


// ============================== Menus defaults =============================

/**
@@md#MenuIntro

Hello ImGui provides a default menu and status bar, which you can customize by using the params:
        `RunnerParams.imGuiWindowParams.` `showMenuBar` / `showMenu_App` / `showMenu_View`

If you want to fully customize the menu:
* set `showMenuBar` to true, then set `showMenu_App` and `showMenu_View` params to false
* implement the callback `RunnerParams.callbacks.ShowMenus`:
  it can optionally call `ShowViewMenu` and `ShowAppMenu` (see below).

@@md
*/
// @@md#MenuFunctions

// `ShowViewMenu(RunnerParams & runnerParams)`:
// shows the View menu (where you can select the layout and docked windows visibility
void ShowViewMenu(RunnerParams & runnerParams);

// `ShowAppMenu(RunnerParams & runnerParams)`:
// shows the default App menu (including the Quit item)
void ShowAppMenu(RunnerParams & runnerParams);
// @@md

}
