#include "hello_imgui/internal/runner_functions.h"
#include "hello_imgui/internal/docking_details.h"
#include "hello_imgui/internal/menu_statusbar.h"

namespace HelloImGui
{
namespace RunnerFunctions
{

void PrepareApp(HelloImGui::RunnerParams & params)
{
    params.callbacks.SetupImGuiConfig();
    params.callbacks.SetupImGuiStyle();

#if not defined(HELLOIMGUI_USE_SOKOL)
    ImGui::GetIO().Fonts->Clear();
    params.callbacks.LoadAdditionalFonts();
    ImGui::GetIO().Fonts->Build();
#else
    #pragma warning("Disabled additional fonts with sokol due to bug!")
#endif

    DockingDetails::ConfigureImGuiDocking(params.imGuiWindowParams);
    if (params.callbacks.PostInit)
        params.callbacks.PostInit();
}

void RenderGui(HelloImGui::RunnerParams & params)
{
    DockingDetails::ProvideWindowOrDock(params.imGuiWindowParams, params.dockingParams);

    if (params.imGuiWindowParams.showMenuBar)
        Menu_StatusBar::ShowMenu(params);

    if (params.callbacks.ShowGui)
        params.callbacks.ShowGui();

    DockingDetails::ShowDockableWindows(params.dockingParams.dockableWindows);

    if (params.imGuiWindowParams.showStatusBar)
        Menu_StatusBar::ShowStatusBar(params);

    DockingDetails::CloseWindowOrDock(params.imGuiWindowParams);
}

void OnPause(HelloImGui::RunnerParams & params)
{
#ifdef HELLOIMGUI_MOBILEDEVICE
    if (params.callbacks.mobileCallbacks.OnPause)
        params.callbacks.mobileCallbacks.OnPause();
#endif
}

void OnResume(HelloImGui::RunnerParams & params)
{
#ifdef HELLOIMGUI_MOBILEDEVICE
    if (params.callbacks.mobileCallbacks.OnResume)
        params.callbacks.mobileCallbacks.OnResume();
#endif
}

void OnDestroy(HelloImGui::RunnerParams & params)
{
#ifdef HELLOIMGUI_MOBILEDEVICE
    if (params.callbacks.mobileCallbacks.OnDestroy)
        params.callbacks.mobileCallbacks.OnDestroy();
#endif
}

void OnLowMemory(HelloImGui::RunnerParams & params)
{
#ifdef HELLOIMGUI_MOBILEDEVICE
    if (params.callbacks.mobileCallbacks.OnLowMemory)
        params.callbacks.mobileCallbacks.OnLowMemory();
#endif
}

} // namespace RunnerFunctions
} // namespace HelloImGui
