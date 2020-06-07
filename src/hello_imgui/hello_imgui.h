#pragma once
#include "hello_imgui/runner_params.h"
#include "hello_imgui/icons_font_awesome.h"

namespace HelloImGui
{
    void Run(RunnerParams & runnerParams);

    void Run(
        GuiFunctionPointer guiFonction,
        ImVec2 windowSize = {800.f, 600.f},
        std::string windowTitle = ""
        );
}
