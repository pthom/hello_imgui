#include "hello_imgui/imgui_default_settings.h"
#include "hello_imgui/icons_font_awesome.h"
#include <string>
#include "imgui.h"

namespace HelloImGui
{
namespace ImGuiDefaultSettings
{
void LoadFontAwesome()
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    ImFontConfig config;
    config.MergeMode = true;
    const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    std::string fontFile = HELLOIMGUI_FONTDIR "/fontawesome-webfont.ttf";
    io.Fonts->AddFontFromFileTTF(fontFile.c_str(), 13.0f, &config, icon_ranges);
    // Usage, e.g.
    //ImGui::Text("%s Search", ICON_FA_SEARCH);
}

void SetupDefaultImGuiConfig()
{
    // Setup Dear ImGui context
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;
}

void SetupDefaultImGuiStyle()
{
    auto& io = ImGui::GetIO();
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows
    // can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
}

}  // namespace ImGuiDefaultSettings
}  // namespace HelloImGui
