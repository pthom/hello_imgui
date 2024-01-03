#pragma once
#include "hello_imgui/runner_params.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/backend_window_helper.h"

namespace HelloImGui
{
    // This function is called by the backend to handle borderless movable windows
    // it returns true if the window should be closed
    bool HandleBorderlessMovable(
        BackendApi::WindowPointer window,
        BackendApi::IBackendWindowHelper * backendWindowHelper,
        const HelloImGui::RunnerParams& runnerParams
    );
}