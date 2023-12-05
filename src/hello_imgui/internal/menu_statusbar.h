#pragma once
#include "hello_imgui/runner_params.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/backend_window_helper.h"
namespace HelloImGui
{
namespace Menu_StatusBar
{
    void ShowMenu(RunnerParams & runnerParams, HelloImGui::BackendApi::WindowPointer window);
    void ShowStatusBar(RunnerParams & params);
}  // namespace Menu_StatusBar
}  // namespace HelloImGui
