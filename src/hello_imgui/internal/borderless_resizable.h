#pragma once
#include "hello_imgui/runner_params.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/backend_window_helper.h"

namespace HelloImGui
{
    void HandleBorderlessResizable(
        BackendApi::WindowPointer window,
        BackendApi::IBackendWindowHelper * backendWindowHelper);
}