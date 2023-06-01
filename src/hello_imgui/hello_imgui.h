#pragma once
#include "hello_imgui/hello_imgui_assets.h"
#include "hello_imgui/hello_imgui_error.h"
#include "hello_imgui/icons_font_awesome.h"
#include "hello_imgui/image_gl.h"
#include "hello_imgui/image_from_asset.h"
#include "hello_imgui/runner_params.h"
#include "hello_imgui/hello_imgui_logger.h"
#include "hello_imgui/imgui_theme.h"
#include "hello_imgui/dpi_aware.h"
#include <string>

#include <cstddef>
#include <cstdint>


/**
@@md#HelloImGui::Run

__HelloImGui::Run()__ will run an application with a single call.

Three signatures are provided:

* `HelloImGui::Run(RunnerParams &)`: full signature, the most customizable version.
   Runs an application whose params and Gui are provided by runnerParams.

* `HelloImGui::Run(const SimpleRunnerParams&)`:
   Runs an application, using simpler params.

* `HelloImGui::Run(guiFunction, windowTitle, windowSize, windowSizeAuto=false, restoreLastWindowGeometry=false, fpsIdle=10)`


__Other utilities:__

* `HelloImGui::GetRunnerParams()`:
  a convenience function that will return the runnerParams of the current application
* `SwitchLayout(layoutName)`
  Changes the application current layout
  (Only used in advanced cases when several layouts are available, i.e. if you filled runnerParams.alternativeDockingLayouts)
* `CurrentLayoutName()`: returns the name of the current layout


@@md
*/
namespace HelloImGui
{
    void Run(RunnerParams & runnerParams);

    void Run(const SimpleRunnerParams& simpleParams);

    void Run(
        const VoidFunction& guiFunction,
        const std::string& windowTitle = "",
        bool windowSizeAuto = false,
        bool windowRestorePreviousGeometry = false,
        const ScreenSize& windowSize = DefaultWindowSize,
        float fpsIdle = 10.f
    );

    RunnerParams* GetRunnerParams();

    void           SwitchLayout(const std::string& layoutName);
    std::string    CurrentLayoutName();

    // Returns the current FrameRate. May differ from ImGui::GetIO().FrameRate,
    // since one can choose the duration for the calculation of the mean value of the fps
    // (Will only lead to accurate values if you call it at each frame)
    float FrameRate(float durationForMean = 0.5f);
}
