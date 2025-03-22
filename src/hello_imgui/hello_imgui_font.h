#pragma once
#include "imgui.h"
#include <vector>
#include <string>


namespace HelloImGui
{
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
        //
        bool adjustSizeToDpi = true;

        // if true, the font will be merged to the last font
        bool mergeToLastFont = false;

        // if true, the font will be loaded using colors
        // (requires freetype, enabled by IMGUI_ENABLE_FREETYPE)
        bool loadColor = false;

        // if true, the font will be loaded using HelloImGui asset system.
        // Otherwise, it will be loaded from the filesystem
        bool insideAssets = true;

        // ImGui native font config to use
        ImFontConfig fontConfig = ImFontConfig();
    };


    // Loads a font with the specified parameters
    ImFont* LoadFont(
        const std::string & fontFilename, float fontSize,
        const FontLoadingParams & params = {});

    ImFont* LoadFontTTF(
        const std::string & fontFilename,
        float fontSize,
        ImFontConfig config = ImFontConfig()
    );

    ImFont* LoadFontTTF_WithFontAwesomeIcons(
        const std::string & fontFilename,
        float fontSize,
        ImFontConfig configFont = ImFontConfig()
    );

    // @@md
}
