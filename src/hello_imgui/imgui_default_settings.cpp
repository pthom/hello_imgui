#include "hello_imgui/icons_font_awesome.h"
#include "hello_imgui/internal/menu_statusbar.h"
#include "imgui.h"
#include <string>

#ifdef IOS
#include "hello_imgui/internal/platform/getAppleBundleResourcePath.h"
#endif
namespace HelloImGui
{
namespace ImGuiDefaultSettings
{

/// Access font files in application bundle or assets/fonts/
std::string fontFileFullPath(const std::string& fontBaseFilename)
{
#ifdef IOS
    std::string path = getAppleBundleResourcePath(fontBaseFilename.c_str());
    return path;
#else
    std::string path = std::string(HELLOIMGUI_ASSETSDIR) + "/fonts/" + fontBaseFilename;
    return path;
#endif
}

void LoadDefaultFont_WithFontAwesome()
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    float fontSize = 14.f;

    //ImFont * font = io.Fonts->AddFontDefault();

    std::string fontFilename = fontFileFullPath("DroidSans.ttf");
    ImFont * font = io.Fonts->AddFontFromFileTTF(fontFilename.c_str(), fontSize);
    assert(font != nullptr); (void)font;
    ImFontConfig config;
    config.MergeMode = true;
    const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

    fontFilename = fontFileFullPath("fontawesome-webfont.ttf");
    font = io.Fonts->AddFontFromFileTTF(fontFilename.c_str(), fontSize, &config, icon_ranges);
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

}  // namespace ImGuiDefaultSettings
}  // namespace HelloImGui
