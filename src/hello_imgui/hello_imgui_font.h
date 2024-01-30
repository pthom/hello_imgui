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
        // This will use an old version of FontAwesome (FontAwesome 4)
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


    // Note: if you want to use a more recent version of Font Awesome,
    // ===============================================================
    // see example at src/hello_imgui_demos/hello_imgui_demo_fontawesome6
    //
    // The principle is summarized below:
    // - Download Font_Awesome_6_Free-Solid-900.otf from https://fontawesome.com/download
    // - Download IconsFontAwesome6.h from https://raw.githubusercontent.com/juliettef/IconFontCppHeaders/main/IconsFontAwesome6.h
    //
    //  Code:
    //
    //    // Prevent HelloImGui from loading Font Awesome 4 definitions, since we will load FontAwesome 6
    //    #define HELLOIMGUI_NO_FONT_AWESOME4
    //    #include "hello_imgui/hello_imgui.h"
    //    #include "IconsFontAwesome6.h"
    //
    //    ...
    //
    //    // Load the default font + merge it with Font Awesome 6
    //    HelloImGui::RunnerParams runnerParams;
    //    runnerParams.callbacks.LoadAdditionalFonts = []
    //    {
    //        // Load the default font
    //        HelloImGui::LoadFont("fonts/DroidSans.ttf", 15.0f);
    //
    //        // Merge FontAwesome6 with the default font
    //        HelloImGui::FontLoadingParams fontParams;
    //        fontParams.mergeToLastFont = true;
    //        fontParams.useFullGlyphRange = true;
    //        HelloImGui::LoadFont("fonts/Font_Awesome_6_Free-Solid-900.otf", 15.0f, fontParams);
    //    };
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