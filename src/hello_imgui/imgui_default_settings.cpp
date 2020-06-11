#include "hello_imgui/imgui_default_behaviors.h"
#include "hello_imgui/icons_font_awesome.h"
#include "imgui.h"
#include <string>

namespace HelloImGui
{
namespace ImGuiDefaultSettings
{
void LoadDefaultFont_WithFontAwesome()
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    float fontSize = 14.f;
    //ImFont * font = io.Fonts->AddFontDefault();
    ImFont * font = io.Fonts->AddFontFromFileTTF(HELLOIMGUI_FONTDIR "/DroidSans.ttf", fontSize);
    assert(font != nullptr); (void)font;
    ImFontConfig config;
    config.MergeMode = true;
    const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    std::string fontFile = HELLOIMGUI_FONTDIR "/fontawesome-webfont.ttf";
    font = io.Fonts->AddFontFromFileTTF(fontFile.c_str(), fontSize, &config, icon_ranges);
    assert(font != nullptr); (void)font;
    io.Fonts->Build();
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

void ShowMenu(RunnerParams & runnerParams)
{
    std::string appName = runnerParams.appWindowParams.windowTitle.c_str();
    ImGui::BeginMenuBar();
    if (ImGui::BeginMenu("Menu name"))
    {
//        IMGUI_API bool          MenuItem(const char* label,
//            const char* shortcut = NULL,
//            bool selected = false,
//            bool enabled = true);
//        // return true when activated. shortcuts are displayed for convenience
//        // but not processed by ImGui at the moment
//        IMGUI_API bool          MenuItem(const char* label,
//            const char* shortcut, bool* p_selected, bool enabled = true);
//        // return true when activated + toggle (*p_selected) if p_selected != NULL

        //const char *nullShortcut = nullptr;
        if (ImGui::MenuItem("Quit"))
            runnerParams.appShallExit = true;

        ImGui::Separator();

        ImGui::EndMenu();
    }
    ImGui::EndMenuBar();

}


}  // namespace ImGuiDefaultSettings
}  // namespace HelloImGui
