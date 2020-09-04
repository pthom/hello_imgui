#pragma once
#include "hello_imgui/runner_params.h"

namespace HelloImGui
{
namespace RunnerFunctions
{
void PrepareApp(HelloImGui::RunnerParams & params);
void RenderGui(HelloImGui::RunnerParams & params);

// Events for mobile devices
void OnPause(HelloImGui::RunnerParams & params);
void OnResume(HelloImGui::RunnerParams & params);
void OnDestroy(HelloImGui::RunnerParams & params);
void OnLowMemory(HelloImGui::RunnerParams & params);

} // namespace RunnerFunctions
} // namespace HelloImGui
