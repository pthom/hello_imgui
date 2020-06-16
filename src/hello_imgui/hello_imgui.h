#pragma once
#include "hello_imgui/runner_params.h"
#include "hello_imgui/icons_font_awesome.h"

/**
@@md

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
        ImVec2 windowSize = {800.f, 600.f},
        std::string windowTitle = ""
        );
}

/**
Warning for SDL apps under iOS:
- SDL will use a dirty hack in order to replace your main() function by its own main()
  which will the  call your own main ! For this reason, we do have to include <SDL_main.h>
- Please make sure that the signature of your main() function is *exactly*
 int main(int argc, char **argv)
*/
#if defined(IOS) && defined(HELLOIMGUI_USE_SDL_OPENGL3)
#include <SDL_main.h>
#endif
