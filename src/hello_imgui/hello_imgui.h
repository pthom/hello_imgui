#pragma once
#include "hello_imgui/hello_imgui_assets.h"
#include "hello_imgui/hello_imgui_error.h"
#include "hello_imgui/icons_font_awesome.h"
#include "hello_imgui/image_gl.h"
#include "hello_imgui/image_from_asset.h"
#include "hello_imgui/runner_params.h"
#include "hello_imgui/hello_imgui_logger.h"
#include <string>
/**
@@md#HelloImGui::Run

__HelloImGui::Run()__ will run an application with a single call.

Three signatures are provided:

* `HelloImGui::Run(RunnerParams &)`: full signature, the most customizable version.
   Runs an application whose params and Gui are provided
by runnerParams.

* `HelloImGui::Run(guiFunction, windowSize, windowTitle, fpsIdle=4)`: signature in order to start a simple application with ease.
  `fpsIdle` enables to set the app FPS when it is idle (set it to 0 for maximum FPS).

* `HelloImGui::Run_AutoSize(guiFunction, windowTitle, restoreLastWindowGeometry = true, fpsIdle=4)`: signature
in order to start a simple application, where the window size can be set automatically from the widgets,
 and where the previous position size & position can be restored upon next launch. `fpsIdle` enables to set
 the app FPS when it is idle (set it to 0 for maximum FPS).

__HelloImGui::GetRunnerParams()__ is a convenience function that will return
the runnerParams of the current application.

 @@md
*/
namespace HelloImGui
{
    void Run(RunnerParams & runnerParams);

    void Run(VoidFunction guiFunction,
        ImVec2 windowSize = ImVec2(800.f, 600.f),
        std::string windowTitle = "",
        float fpsIdle = 10.f
        );

    void Run_AutoSize(VoidFunction guiFunction,
             std::string windowTitle = "",
             bool restoreLastWindowGeometry = true,
             ImVec2 windowSize = ImVec2(0.f, 0.f),
             float fpsIdle = 10.f
             );

    RunnerParams* GetRunnerParams();
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
