#define IMGUI_DEFINE_MATH_OPERATORS
#include "hello_imgui/internal/menu_statusbar.h"
#include "hello_imgui/internal/docking_details.h"

#include "imgui.h"
#include "hello_imgui/internal/imgui_global_context.h" // must be included before imgui_internal.h
#include "hello_imgui/hello_imgui.h"
#include "imgui_internal.h"

#include <string>

namespace HelloImGui
{
bool ShouldRemoteDisplay(); // from hello_imgui.cpp

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
    if (!runnerParams.imGuiWindowParams.showMenuBar)
        return;
    ImGui::BeginMainMenuBar();

    if (runnerParams.imGuiWindowParams.showMenu_App)
        ShowDefaultAppMenu_Quit(runnerParams);

    if (runnerParams.imGuiWindowParams.showMenu_View)
        DockingDetails::ShowViewMenu(runnerParams);

    if (runnerParams.callbacks.ShowMenus)
        runnerParams.callbacks.ShowMenus();

    ImGui::EndMainMenuBar();
}


void ShowStatusBar(RunnerParams & params)
{
    float statusWindowHeight = ImGui::GetFrameHeight() * 1.4f;
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImVec2 statusBarSize, statusBarPos;
    {
        // One some platform, like iOS, we need to take into account the insets
        // so that our app does not go under the notch or the home indicator
        EdgeInsets edgeInsets;
        if (params.appWindowParams.handleEdgeInsets)
            edgeInsets = params.appWindowParams.edgeInsets;
        statusBarSize = ImVec2(viewport->Size.x - (float)edgeInsets.left - (float)edgeInsets.right, statusWindowHeight);
        statusBarPos = viewport->Pos + ImVec2((float)edgeInsets.left, viewport->Size.y - (float)edgeInsets.bottom - statusBarSize.y);
    }

    ImGui::SetNextWindowPos(statusBarPos);
    ImGui::SetNextWindowSize(statusBarSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking;
    ImGui::Begin("StatusBar", nullptr, windowFlags);

    if (params.callbacks.ShowStatus)
        params.callbacks.ShowStatus();

    if (params.imGuiWindowParams.showStatus_Fps)
    {
		if (ShouldRemoteDisplay())
		{
			ImGui::SameLine(ImGui::GetIO().DisplaySize.x - 5.f * ImGui::GetFontSize());
			ImGui::Text("FPS: %.1f", HelloImGui::FrameRate());
		}
		else
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
    }

    ImGui::End();
}

}  // namespace Menu_StatusBar
}  // namespace HelloImGui
