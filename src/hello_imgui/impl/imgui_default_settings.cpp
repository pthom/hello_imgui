#include "imgui_default_settings.h"
#include "hello_imgui_assets.h"
#include "hello_imgui_font.h"


namespace HelloImGui
{


namespace ImGuiDefaultSettings
{


void LoadDefaultFont_WithFontAwesomeIcons()
{
    std::string fontFilename = "fonts/DroidSans.ttf";
    if (HelloImGui::AssetExists(fontFilename))
    {
        FontLoadingParams fontLoadingParams;
        fontLoadingParams.mergeFontAwesome = true;
        ImFont* font = LoadFont(fontFilename, 15.f, fontLoadingParams);
        (void) font;
    }
    else
    {
        ImGui::GetIO().Fonts->AddFontDefault();
    }
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
