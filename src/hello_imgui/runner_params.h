#pragma once
#include "hello_imgui/app_window_params.h"
#include "hello_imgui/imgui_window_params.h"
#include "hello_imgui/internal/main_screen_resolution.h"
#include "hello_imgui/runner_callbacks.h"

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
    AppWindowParams appWindowParams;
    /// imgui window params (use docking, showMenuBar, ProvideFullScreenWindow, etc)
    ImGuiWindowParams imGuiWindowParams;
};

}  // namespace HelloImGui