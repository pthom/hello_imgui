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

    Two signatures are provided:

    * `HelloImGui::Run(RunnerParams &)`: full signature, the most customizable version.
       Runs an application whose params and Gui are provided
    by runnerParams.

    * `HelloImGui::Run(guiFunction, windowSize, windowTitle)`: simple signature
    in order to start a simple application with ease.

@@md
*/
namespace HelloImGui
{
    void Run(RunnerParams & runnerParams);

    void Run(VoidFunction guiFonction,
        ImVec2 windowSize = ImVec2(800.f, 600.f),
        std::string windowTitle = ""
        );
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
