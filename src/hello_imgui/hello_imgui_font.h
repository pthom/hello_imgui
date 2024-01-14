#pragma once
#include "imgui.h"
#include <string>
#include <vector>
#include <array>

namespace HelloImGui
{
    using ImWcharPair = std::array<ImWchar, 2>;

    // @@md#Fonts
    //
    // When loading fonts, use HelloImGui::LoadFont(fontFilename, fontSize, fontLoadingParams)
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

        // the ranges of glyphs to load:
        //    - if empty, the default glyph range will be used
        //    - you can specify several ranges
        //    - intervals bounds are inclusive
        // (will be translated and stored as a static ImWChar* inside fontConfig)
        std::vector<ImWcharPair> glyphRanges = {};

        // ImGui native font config to use
        ImFontConfig fontConfig = ImFontConfig();

        // if true, the font will be loaded and then FontAwesome icons will be merged to it
        // (deprecated, use mergeToLastFont instead, and load in two steps)
        bool mergeFontAwesome = false;
        ImFontConfig fontConfigFontAwesome = ImFontConfig();
    };

    // When loading fonts, use HelloImGui::LoadFont(FontLoadingParams)
    // ===============================================================
    // instead of ImGui::GetIO().Fonts->AddFontFromFileTTF(), because it will
    // automatically adjust the font size to account for HighDPI, and will spare
    // you headaches when trying to get consistent font size across different OSes.
    // see FontLoadingParams and ImFontConfig
    ImFont* LoadFont(const std::string & fontFilename, float fontSize,
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