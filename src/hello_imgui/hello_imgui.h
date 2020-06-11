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
