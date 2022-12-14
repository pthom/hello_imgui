#pragma once
#include "hello_imgui/hello_imgui_assets.h"
#include "hello_imgui/hello_imgui_error.h"
#include "hello_imgui/icons_font_awesome.h"
#include "hello_imgui/image_gl.h"
#include "hello_imgui/image_from_asset.h"
#include "hello_imgui/runner_params.h"
#include "hello_imgui/hello_imgui_logger.h"
#include "hello_imgui/imgui_theme.h"
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

__HelloImGui::GetRunnerParams()__ is a convenience function that will return the runnerParams of the current application.

__HelloImGui::EmSize()__ returns the visible font size on the screen. For reproducible results, even on HighDPI screens,
 always scale your widgets and windows relatively to this size. It is somewhat comparable to the
 [em CSS Unit](https://lyty.dev/css/css-unit.html).
_EmSize() = ImGui::GetFontSize() / ImGui::GetIO().FontGlobalScale_ (on MacOS FontGlobalScale can be = 2.0).

__HelloImGui::EmSize(nbLines)__ returns a size corresponding to nbLines text lines

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

    // __HelloImGui::EmSize()__ returns the visible font size on the screen. For good results on HighDPI screens, always scale your
    // widgets and windows relatively to this size.
    // It is somewhat comparable to the [em CSS Unit](https://lyty.dev/css/css-unit.html).
    // EmSize() = ImGui::GetFontSize()
    float EmSize();

    // __HelloImGui::EmSize(nbLines)__ returns a size corresponding to nbLines text lines
    float EmSize(float nbLines);

    // __HelloImGui::EmVec2()__ returns an ImVec2 that you can use to size or place your widgets in a DPI independent way
    ImVec2 EmVec2(float x, float y);
    ImVec2 EmVec2(ImVec2 v);
}

/**
@@md#SDLMain

Warning for SDL apps under iOS and Android:

SDL uses a dirty hack in order to _replace your main() function by its own main() function_,
which will then call your own main !

Please make sure that the signature of your main() function is *exactly*
    `int main(int argc, char **argv)`
and that your main() function returns an int.

@@md
*/
#if defined(HELLOIMGUI_USE_SDL_OPENGL3)
#include <SDL_main.h>
#endif
