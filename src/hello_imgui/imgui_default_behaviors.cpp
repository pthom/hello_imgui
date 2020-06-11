#include "hello_imgui/imgui_default_behaviors.h"
#include "hello_imgui/icons_font_awesome.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <string>

namespace HelloImGui
{
namespace ImGuiDefaultBehaviors
{

void MenuItems_FontScale()
{
    float ratio = ImGui::GetIO().FontGlobalScale;
    char msgCurrentRatio[1000];
    snprintf(msgCurrentRatio, 1000, "Font Scale: %.1f", ratio);
    ImGui::MenuItem(msgCurrentRatio, nullptr, false, false);

    bool zoomChanged = false;
    if (ImGui::MenuItem("Zoom ++"))
    {
        ratio = ratio * 1.2f * 1.2f;
        zoomChanged = true;
    }
    if (ImGui::MenuItem("Zoom +"))
    {
        ratio = ratio * 1.2f;
        zoomChanged = true;
    }
    if (ImGui::MenuItem("Zoom -"))
    {
        ratio = ratio / 1.2f;
        zoomChanged = true;
    }
    if (ImGui::MenuItem("Zoom --"))
    {
        ratio = ratio / 1.2f / 1.2f;
        zoomChanged = true;
    }
    if (ImGui::MenuItem("Restore Zoom"))
    {
        ratio = 1.f;
        zoomChanged = true;
    }

    if (zoomChanged)
        ImGui::GetIO().FontGlobalScale = ratio;

}

void ShowMenu(RunnerParams & runnerParams)
{
    bool hasMenu = ImGui::GetCurrentWindow()->Flags & ImGuiWindowFlags_MenuBar;
    if (!hasMenu)
        return;

    std::string appName = runnerParams.appWindowParams.windowTitle.c_str();
    if (appName.empty())
        appName = "App";
    ImGui::BeginMenuBar();
    if (ImGui::BeginMenu(appName.c_str()))
    {
        ImGui::Separator();
        MenuItems_FontScale();
        ImGui::Separator();

        if (ImGui::MenuItem("Quit"))
            runnerParams.appShallExit = true;

        ImGui::EndMenu();
    }
    ImGui::EndMenuBar();

}

}  // namespace ImGuiDefaultBehaviors
}  // namespace HelloImGui
