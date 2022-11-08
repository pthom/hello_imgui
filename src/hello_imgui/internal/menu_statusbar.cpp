#include "hello_imgui/internal/menu_statusbar.h"
#include "hello_imgui/internal/docking_details.h"

#include "imgui.h"
#include "hello_imgui/internal/imgui_global_context.h" // must be included before imgui_internal.h
#include "imgui_internal.h"

#include <string>

namespace HelloImGui
{
namespace Menu_StatusBar
{

void ShowDefaultAppMenu_Quit(RunnerParams & runnerParams)
{
    std::string appName = runnerParams.appWindowParams.windowTitle.c_str();
    if (appName.empty())
        appName = "App";

    bool hideMenuBecauseEmpty = false;
#ifdef HELLOIMGUI_CANNOTQUIT
    hideMenuBecauseEmpty = true;
#endif
    if (hideMenuBecauseEmpty)
        return;

    if (ImGui::BeginMenu(appName.c_str()))
    {
#ifndef HELLOIMGUI_CANNOTQUIT
        if (ImGui::MenuItem( "Quit"))
            runnerParams.appShallExit = true;
#endif
        ImGui::EndMenu();
    }

}

void ShowMenu(RunnerParams & runnerParams)
{
    bool hasMenu = (ImGui::GetCurrentWindow()->Flags & ImGuiWindowFlags_MenuBar) != 0;
    if (!hasMenu)
        return;

    ImGui::BeginMenuBar();

    if (runnerParams.imGuiWindowParams.showMenu_App)
        ShowDefaultAppMenu_Quit(runnerParams);

    if (runnerParams.imGuiWindowParams.showMenu_View)
        DockingDetails::ShowViewMenu(runnerParams);

    if (runnerParams.callbacks.ShowMenus)
        runnerParams.callbacks.ShowMenus();

    ImGui::EndMenuBar();
}


void ShowStatusBar(const RunnerParams & params)
{
    float statusWindowHeight = ImGui::GetFrameHeight() * 1.2f;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - statusWindowHeight));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, statusWindowHeight));
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking;
    ImGui::Begin("StatusBar", nullptr, windowFlags);

    if (params.callbacks.ShowStatus)
        params.callbacks.ShowStatus();

    if (params.imGuiWindowParams.showStatus_Fps)
    {
        ImGui::SameLine(ImGui::GetIO().DisplaySize.x - 5.f * ImGui::GetFontSize());
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    }

    ImGui::End();
}


}  // namespace Menu_StatusBar
}  // namespace HelloImGui
