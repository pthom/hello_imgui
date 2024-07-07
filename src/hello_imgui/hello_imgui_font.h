#pragma once
#include "imgui.h"
#include <string>
#include <vector>
#include <array>

namespace HelloImGui
{
    using ImWcharPair = std::array<ImWchar, 2>;

    // Utility to translate DearImGui common Unicode ranges to ImWcharPair (Python)
    //   (get_glyph_ranges_chinese_simplified_common, get_glyph_ranges_japanese, ...)
    std::vector<ImWcharPair> translate_common_glyph_ranges(const std::vector<ImWchar> & glyphRanges);

    // Utility to translate DearImGui common Unicode ranges to ImWcharPair (C++)
    //   (GetGlyphRangesChineseSimplifiedCommon, GetGlyphRangesJapanese, ...)
    std::vector<ImWcharPair> TranslateCommonGlyphRanges(const ImWchar* glyphRanges);

    // @@md#Fonts

    // When loading fonts, use
    //          HelloImGui::LoadFont(..)
    //      or
    //      	HelloImGui::LoadDpiResponsiveFont()
    //
    // Use these functions instead of ImGui::GetIO().Fonts->AddFontFromFileTTF(),
    // because they will automatically adjust the font size to account for HighDPI,
    // and will help you to get consistent font size across different OSes.

    //
    // Font loading parameters: several options are available (color, merging, range, ...)
    struct FontLoadingParams
    {
        // if true, the font size will be adjusted automatically to account for HighDPI
        bool adjustSizeToDpi = true;

        // if true, the font will be loaded with the full glyph range
        bool useFullGlyphRange = false;
        // if set, fontConfig.GlyphRanges, and
        //   fontConfig.OversampleH / fontConfig.OversampleV will be set to 1
        //   when useFullGlyphRange is true (this is useful to save memory)
        bool reduceMemoryUsageIfFullGlyphRange = true;

        // if true, the font will be merged to the last font
        bool mergeToLastFont = false;

        // if true, the font will be loaded using colors
        // (requires freetype, enabled by IMGUI_ENABLE_FREETYPE)
        bool loadColor = false;

        // if true, the font will be loaded using HelloImGui asset system.
        // Otherwise, it will be loaded from the filesystem
        bool insideAssets = true;

        // the ranges of glyphs to load, as a list of pairs of ImWchar
        //    - if empty, the default glyph range will be used
        //    - you can specify several ranges
        //    - intervals bounds are inclusive
        // Note: in order to use common ranges defined by ImGui (GetGlyphRangesJapanese, GetGlyphRangesChinese, ...)
        //       use TranslateCommonGlyphRanges (or translate_common_glyph_ranges in Python)
        std::vector<ImWcharPair> glyphRanges = {};

        // ImGui native font config to use
        ImFontConfig fontConfig = ImFontConfig();

        // if true, the font will be loaded and then FontAwesome icons will be merged to it
        // (deprecated, use mergeToLastFont instead, and load in two steps)
        // This will use an old version of FontAwesome (FontAwesome 4)
        bool mergeFontAwesome = false;
        ImFontConfig fontConfigFontAwesome = ImFontConfig();
    };

    // A font that will be automatically resized to account for changes in DPI
    // Use LoadAdaptiveFont instead of LoadFont to get this behavior.
    // Fonts loaded with LoadAdaptiveFont will be reloaded during execution
    // if ImGui::GetIO().FontGlobalScale is changed.
    struct FontDpiResponsive
    {
        ImFont* font = nullptr;
        std::string fontFilename;
        float fontSize = 0.f;
        FontLoadingParams fontLoadingParams;
    };


    // Loads a font with the specified parameters
    // (this font will not adapt to DPI changes after startup)
    ImFont* LoadFont(
        const std::string & fontFilename, float fontSize,
        const FontLoadingParams & params = {});

    // Loads a font with the specified parameters
    // This font will adapt to DPI changes after startup.
    // Only fonts loaded with LoadAdaptiveFont will adapt to DPI changes:
    // avoid mixing LoadFont/LoadFontDpiResponsive)
    FontDpiResponsive* LoadFontDpiResponsive(
        const std::string & fontFilename, float fontSize,
        const FontLoadingParams & params = {});

    // @@md

    //
    // Deprecated API below, kept for compatibility (uses LoadFont internally)
    //
    ImFont* LoadFontTTF(
        const std::string & fontFilename,
        float fontSize,
        bool useFullGlyphRange = false,
        ImFontConfig config = ImFontConfig()
        );
    ImFont* LoadFontTTF_WithFontAwesomeIcons(
        const std::string & fontFilename,
        float fontSize,
        bool useFullGlyphRange = false,
        ImFontConfig configFont = ImFontConfig(),
        ImFontConfig configIcons = ImFontConfig()
        );
    ImFont* MergeFontAwesomeToLastFont(float fontSize, ImFontConfig config = ImFontConfig());


    // indicates that fonts were loaded using HelloImGui::LoadFont. In that case, fonts may have been resized to
    // account for HighDPI (on macOS and emscripten)
    bool DidCallHelloImGuiLoadFontTTF();
}