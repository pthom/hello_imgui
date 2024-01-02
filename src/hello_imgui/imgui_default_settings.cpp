#include "hello_imgui/icons_font_awesome.h"
#include "hello_imgui/internal/menu_statusbar.h"
#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/hello_imgui_assets.h"
#include "imgui.h"
#ifdef IMGUI_ENABLE_FREETYPE
#include "misc/freetype/imgui_freetype.h"
#endif

#include <string>
#include <map>
#include <stdlib.h>
#include <stdio.h>

#ifdef IOS
#include "hello_imgui/internal/platform/getAppleBundleResourcePath.h"
#endif


namespace HelloImGui
{
bool gDidCallHelloImGuiLoadFontTTF = false;

ImFont* LoadFontTTF(const std::string & fontFilename, float fontSize, bool useFullGlyphRange, ImFontConfig config)
{
    if (!HelloImGui::AssetExists(fontFilename))
    {
        fprintf(stderr, "Cannot find font %s\n", fontFilename.c_str());
        return nullptr;
    }
    AssetFileData fontData = LoadAssetFileData(fontFilename.c_str());

    config.FontDataOwnedByAtlas = false;
    if (useFullGlyphRange) {
        static ImWchar glyphRange[3] = { 0x20, 0xFFFF, 0 };
        config.GlyphRanges = glyphRange;
    }

    float fontSizeAdjusted = HelloImGui::DpiFontLoadingFactor() * fontSize;
    ImFont * font = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(fontData.data, (int)fontData.dataSize, fontSizeAdjusted, &config);
    if (font == nullptr)
        fprintf(stderr, "Cannot load font %s\n", fontFilename.c_str());
    FreeAssetFileData(&fontData);
    gDidCallHelloImGuiLoadFontTTF = true;
    return font;
}

ImFont* LoadFontTTF_WithFontAwesomeIcons(const std::string & fontFilename, float fontSize, bool useFullGlyphRange, ImFontConfig configFont, ImFontConfig configIcons)
{
    ImFont *font = LoadFontTTF(fontFilename, fontSize, useFullGlyphRange, configFont);
    if (font == nullptr)
        return nullptr;
    font = MergeFontAwesomeToLastFont(fontSize, configIcons);
    return font;
}

#ifdef IMGUI_ENABLE_FREETYPE
ImFont* LoadEmojiFont(const std::string & fontFilename, float fontSize, bool mergeToLastFont)
{
    if (!HelloImGui::AssetExists(fontFilename))
    {
        fprintf(stderr, "Cannot find font %s\n", fontFilename.c_str());
        return nullptr;
    }
    AssetFileData fontData = LoadAssetFileData(fontFilename.c_str());

    static ImWchar fullGlyphRange[] =
        //{ 0X1F600, 0X1F64F, 0}; // emoticons
        { 0x1, 0x1FFFF, 0 };

    auto createEmojiConfig = []() -> ImFontConfig {
        ImFontConfig cfg;
        cfg.FontDataOwnedByAtlas = false;
        cfg.OversampleH = cfg.OversampleV = 1;
        cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
        return cfg;
    };

    static ImFontConfig fontConfigNoMerge = createEmojiConfig(), fontConfigMerge = createEmojiConfig();
    fontConfigMerge.MergeMode = true;
    fontConfigNoMerge.MergeMode = false;
    ImFontConfig *currentFontConfig = mergeToLastFont ? &fontConfigMerge : &fontConfigNoMerge;

    float fontSizeAdjusted = HelloImGui::DpiFontLoadingFactor() * fontSize;
    ImFont * font = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(
        fontData.data, (int)fontData.dataSize,
        fontSizeAdjusted,
        currentFontConfig,
        fullGlyphRange
        );
    if (font == nullptr)
        fprintf(stderr, "Cannot load font %s\n", fontFilename.c_str());
    FreeAssetFileData(&fontData);
    gDidCallHelloImGuiLoadFontTTF = true;
    return font;
}
#endif

ImFont* MergeFontAwesomeToLastFont(float fontSize, ImFontConfig config)
{
    static std::string faFile = "fonts/fontawesome-webfont.ttf";

    AssetFileData fontData = LoadAssetFileData(faFile.c_str());

    static const ImWchar icon_fa_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    config.MergeMode = true;
    config.FontDataOwnedByAtlas = false;
    float fontSizeAdjusted = HelloImGui::DpiFontLoadingFactor() * fontSize;

    auto font = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(
        fontData.data, (int)fontData.dataSize, fontSizeAdjusted, &config, icon_fa_ranges);

    if (font == nullptr)
        HIMG_ERROR(std::string("Cannot load ") + faFile);
    FreeAssetFileData(&fontData);
    return font;
}


namespace ImGuiDefaultSettings
{

bool DidCallHelloImGuiLoadFontTTF()
{
    return HelloImGui::gDidCallHelloImGuiLoadFontTTF;
}

void LoadDefaultFont_WithFontAwesomeIcons()
{
    std::string fontFilename = "fonts/DroidSans.ttf";
    if (HelloImGui::AssetExists(fontFilename))
    {
        float fontSize = 15.f;
        ImFont* font = LoadFontTTF_WithFontAwesomeIcons(fontFilename, fontSize, false);
        (void)font;
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
