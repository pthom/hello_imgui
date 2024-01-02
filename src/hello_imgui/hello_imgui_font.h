#pragma once
#include "imgui.h"
#include <string>

namespace HelloImGui
{
    // Loads a font from a file, not using the asset system
    // This version tries to correct several issues with ImGui default font loading mechanism:
    //     - the font size will be adjusted automatically to account for HighDPI
    //     - you can pass glyph ranges as a vector of ranges (ImVector<ImWchar[2]>), and not care about its lifetime
    //     - you can pass a font config (ImFontConfig) and not care about its lifetime
    ImFont* LoadFontFromFileTTF(
        const std::string& filename,
        float fontSize,
        bool mergeToLastFont = false,
        const ImFontConfig& fontConfig = ImFontConfig(),
        const ImVector<ImWchar[2]> & glyphRanges = {}
        );

    // Loads a font from a file, using the asset system
    // This version tries to correct several issues with ImGui default font loading mechanism:
    //     - the font size will be adjusted automatically to account for HighDPI
    //     - you can pass glyph ranges as a vector of ranges (ImVector<ImWchar[2]>), and not care about its lifetime
    //     - you can pass a font config (ImFontConfig) and not care about its lifetime
    ImFont* LoadFontFromAssetFileTTF(
        const std::string& filename,
        float fontSize,
        bool mergeToLastFont = false,
        const ImFontConfig& fontConfig = ImFontConfig(),
        const ImVector<ImWchar[2]> & glyphRanges = {}
    );

    // Uses LoadFontFromAssetFileTTF internally.
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

    #ifdef IMGUI_ENABLE_FREETYPE
    ImFont* LoadEmojiFontFromAssetFileTTF(
        const std::string & fontFilename,
        float fontSize,
        bool mergeToLastFont,
        const ImVector<ImWchar[2]> & glyph_ranges = {}
        );
    #endif


    ImFont* MergeFontAwesomeToLastFont(float fontSize, ImFontConfig config = ImFontConfig());

    // indicates that fonts were loaded using HelloImGui::LoadFontTTF. In that case, fonts may have been resized to
    // account for HighDPI (on macOS and emscripten)
    bool DidCallHelloImGuiLoadFontTTF();
}