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


namespace HelloImGui
{

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

* `FrameRate(durationForMean = 0.5)`: Returns the current FrameRate.
  May differ from ImGui::GetIO().FrameRate, since one can choose the duration for the calculation of the mean value of the fps
@@md
*/
    void Run(RunnerParams &runnerParams);

    void Run(const SimpleRunnerParams &simpleParams);

    void Run(
        const VoidFunction &guiFunction,
        const std::string &windowTitle = "",
        bool windowSizeAuto = false,
        bool windowRestorePreviousGeometry = false,
        const ScreenSize &windowSize = DefaultWindowSize,
        float fpsIdle = 10.f
    );

    RunnerParams *GetRunnerParams();

    // Returns the current FrameRate. May differ from ImGui::GetIO().FrameRate,
    // since one can choose the duration for the calculation of the mean value of the fps
    // (Will only lead to accurate values if you call it at each frame)
    float FrameRate(float durationForMean = 0.5f);


/**
@@md#HelloImGui::Layouts

 In advanced cases when several layouts are available, you can switch between layouts.
(see demo inside [hello_imgui_demodocking.main.cpp](../hello_imgui_demos/hello_imgui_demodocking/hello_imgui_demodocking.main.cpp))

* `SwitchLayout(layoutName)`
  Changes the application current layout. Only used in advanced cases when several layouts are available,
  i.e. if you filled runnerParams.alternativeDockingLayouts.
* `CurrentLayoutName()`: returns the name of the current layout
@@md
*/
    void           SwitchLayout(const std::string& layoutName);
    std::string    CurrentLayoutName();


/**
@@md#HelloImGui::UserPref

You may store additional user settings in the application settings. This is provided as a convenience only,
and it is not intended to store large quantities of text data. Use sparingly.

* `SaveUserPref(string userPrefName, string userPrefContent)`:
  Shall be called in the callback runnerParams.callbacks.BeforeExit

* `string LoadUserPref(string& userPrefName)`
  Shall be called in the callback runnerParams.callbacks.PostInit
@@md
*/
    void        SaveUserPref(const std::string& userPrefName, const std::string& userPrefContent);
    std::string LoadUserPref(const std::string& userPrefName);
}
