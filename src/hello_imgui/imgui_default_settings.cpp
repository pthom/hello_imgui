#include "hello_imgui/icons_font_awesome.h"
#include "hello_imgui/internal/menu_statusbar.h"
#include "hello_imgui/hello_imgui.h"
#include "imgui.h"
#include <string>
#include <stdlib.h>
#ifdef IOS
#include "hello_imgui/internal/platform/getAppleBundleResourcePath.h"
#endif

namespace HelloImGui
{
ImFont* LoadFontTTF(const std::string & fontFilename, float fontSize)
{
    auto file_exists = [](const std::string &filename) {
        FILE * f = fopen(filename.c_str(), "r");
        bool exists = (f != NULL);
        if (exists)
            fclose(f);
        return  exists;
    };

    if (!file_exists(fontFilename))
        HIMG_THROW_STRING(std::string("LoadFontTTF, file not found: ") + fontFilename);

    ImFont * font = ImGui::GetIO().Fonts->AddFontFromFileTTF(fontFilename.c_str(), fontSize);
    if (font == nullptr) {
        HIMG_THROW_STRING(std::string("Cannot load ") + fontFilename);
    }
    return font;
};

ImFont* MergeFontAwesomeToLastFont(float fontSize)
{
    static std::string faFile = HelloImGui::assetFileFullPath("fonts/fontawesome-webfont.ttf");
    static const ImWchar icon_fa_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    static ImFontConfig faConfig = [] {
        ImFontConfig config;
        config.MergeMode = true;
        return config;
    }();
    auto font = ImGui::GetIO().Fonts->AddFontFromFileTTF(faFile.c_str(), fontSize, &faConfig, icon_fa_ranges);
    if (font == nullptr)
        HIMG_THROW_STRING(std::string("Cannot load ") + faFile);
    return font;
}

ImFont* LoadFontTTF_WithFontAwesomeIcons(const std::string & fontFilename, float fontSize)
{
    ImFont *font = LoadFontTTF(fontFilename, fontSize);
    font = MergeFontAwesomeToLastFont(fontSize);
    return font;
}

namespace ImGuiDefaultSettings
{


void LoadDefaultFont_WithFontAwesomeIcons()
{
    float fontSize = 14.f;
    std::string fontFilename = assetFileFullPath("fonts/DroidSans.ttf");
    ImFont * font = LoadFontTTF(fontFilename, fontSize);
    (void) font;
    MergeFontAwesomeToLastFont(fontSize);
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
