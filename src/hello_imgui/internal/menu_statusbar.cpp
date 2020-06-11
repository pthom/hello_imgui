#include "hello_imgui/internal/menu_statusbar.h"
#include "hello_imgui/internal/docking_details.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <string>

namespace HelloImGui
{
namespace Menu_StatusBar
{

void MenuItems_FontScale()
{
    float ratio = ImGui::GetIO().FontGlobalScale;
    char msgCurrentRatio[1000];
    snprintf(msgCurrentRatio, 1000, "Font Scale: %.1f", ratio);
    ImGui::MenuItem(msgCurrentRatio, nullptr, false, false);

    bool zoomChanged = false;
    if (ImGui::MenuItem( "Zoom ++"))
    {
        ratio = ratio * 1.2f * 1.2f;
        zoomChanged = true;
    }
    if (ImGui::MenuItem( "Zoom +"))
    {
        ratio = ratio * 1.2f;
        zoomChanged = true;
    }
    if (ImGui::MenuItem( "Zoom -"))
    {
        ratio = ratio / 1.2f;
        zoomChanged = true;
    }
    if (ImGui::MenuItem(  "Zoom --"))
    {
        ratio = ratio / 1.2f / 1.2f;
        zoomChanged = true;
    }
    if (ImGui::MenuItem( "Restore Zoom"))
    {
        ratio = 1.f;
        zoomChanged = true;
    }

    if (zoomChanged)
        ImGui::GetIO().FontGlobalScale = ratio;

}

void ShowDefaultAppMenu_QuitZoom(RunnerParams & runnerParams)
{
    std::string appName = runnerParams.appWindowParams.windowTitle.c_str();
    if (appName.empty())
        appName = "App";

    if (ImGui::BeginMenu(appName.c_str()))
    {
        ImGui::Separator();
        MenuItems_FontScale();
        ImGui::Separator();

        if (ImGui::MenuItem( "Quit"))
            runnerParams.appShallExit = true;

        ImGui::EndMenu();
    }

}

void ShowMenu(RunnerParams & runnerParams)
{
    bool hasMenu = ImGui::GetCurrentWindow()->Flags & ImGuiWindowFlags_MenuBar;
    if (!hasMenu)
        return;

    ImGui::BeginMenuBar();

    if (runnerParams.imGuiWindowParams.showMenu_App_QuitZoom)
        ShowDefaultAppMenu_QuitZoom(runnerParams);

    if (runnerParams.imGuiWindowParams.showMenu_View)
        DockingDetails::ShowViewMenu(runnerParams);

    if (runnerParams.callbacks.ShowMenus)
        runnerParams.callbacks.ShowMenus();

    ImGui::EndMenuBar();
}

void ShowStatus_Fps()
{
    ImGui::SameLine(ImGui::GetIO().DisplaySize.x - 90.f);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
}

void ShowStatusBar(const RunnerParams & params)
{
    float statusWindowHeight = 30.f;
    ImVec2 windowLocation( 0., ImGui::GetIO().DisplaySize.y - statusWindowHeight );
    ImVec2 windowSize(ImGui::GetIO().DisplaySize.x, statusWindowHeight);
    ImGui::SetNextWindowPos(windowLocation);
    ImGui::SetNextWindowSize(windowSize);
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking;
    ImGui::Begin("StatusBar", nullptr, windowFlags);

    if (params.callbacks.ShowStatus)
        params.callbacks.ShowStatus();

    if (params.imGuiWindowParams.showStatus_Fps)
        ShowStatus_Fps();

    ImGui::End();
}


}  // namespace Menu_StatusBar
}  // namespace HelloImGui
