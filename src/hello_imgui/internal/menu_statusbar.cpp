#include "hello_imgui/internal/menu_statusbar.h"
#include "hello_imgui/internal/docking_details.h"

#include "imgui.h"
#include "hello_imgui/internal/imgui_global_context.h" // must be included before imgui_internal.h
#include "hello_imgui/hello_imgui.h"
#include "imgui_internal.h"

#include <string>

namespace HelloImGui
{
namespace Menu_StatusBar
{

void ShowDefaultAppMenu_Quit(RunnerParams & runnerParams)
{
    std::string menuAppTitle = runnerParams.imGuiWindowParams.menuAppTitle;
    if (menuAppTitle.empty())
        menuAppTitle = runnerParams.appWindowParams.windowTitle;
    if (menuAppTitle.empty())
        menuAppTitle = "App";

#ifdef HELLOIMGUI_CANNOTQUIT
    const bool displayQuitItem = false;
#else
    const bool displayQuitItem = runnerParams.imGuiWindowParams.showMenu_App_Quit;
#endif

    const bool hasCustomAppMenuItems = static_cast<bool>(runnerParams.callbacks.ShowAppMenuItems);

    const bool isAppMenuEmpty = !displayQuitItem && !hasCustomAppMenuItems;
    if (isAppMenuEmpty)
        return;

    if (ImGui::BeginMenu(menuAppTitle.c_str()))
    {
        if (runnerParams.callbacks.ShowAppMenuItems)
            runnerParams.callbacks.ShowAppMenuItems();

        if (displayQuitItem)
        {
            if (hasCustomAppMenuItems)
                ImGui::Separator();
            if (ImGui::MenuItem( "Quit"))
                runnerParams.appShallExit = true;
        }
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


void ShowStatusBar(RunnerParams & params)
{
    float statusWindowHeight = ImGui::GetFrameHeight() * 1.4f;
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
        float dy = ImGui::GetFontSize() * 0.15f;

        ImGui::SameLine(ImGui::GetIO().DisplaySize.x - 14.f * ImGui::GetFontSize());

        const char* idlingInfo = params.fpsIdling.isIdling ? " (Idling)" : "";

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - dy); // The checkbox seems visually misaligned, let's fix this
        ImGui::Checkbox("Enable idling", &params.fpsIdling.enableIdling);
        ImGui::SameLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - dy);
        ImGui::Text("FPS: %.1f%s", HelloImGui::FrameRate(), idlingInfo);
    }

    ImGui::End();
}


}  // namespace Menu_StatusBar
}  // namespace HelloImGui
