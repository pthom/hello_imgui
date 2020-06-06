#pragma once
#include "backend_window_params.h"
#include "imgui_window_params.h"
#include "internal/main_screen_resolution.h"
#include "runner_callbacks.h"

namespace HelloImGui
{

/**
 * RunnerParams contains all the information and callbacks
 * needed to run an application
 */
struct RunnerParams
{
    /// RunnerCallbacks (ShowGui() will render the gui, Config, etc)
    RunnerCallbacks callbacks;
    /// application Window Params (position, size, title)
    BackendWindowParams backendWindowParams;
    /// imgui window params (use docking, ShowMenuBar, ProvideFullScreenWindow, etc)
    ImGuiWindowParams imGuiWindowParams;
};
}  // namespace HelloImGui