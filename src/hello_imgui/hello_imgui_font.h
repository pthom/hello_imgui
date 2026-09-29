#pragma once
#include "imgui.h"
#include <vector>
#include <string>


namespace HelloImGui
{
    // ::code Fonts

    // Font loading
    //
    // HelloImGui::LoadFont() loads a font from the assets folder, which exists on every platform
    // (desktop, mobile, browser), where ImGui::GetIO().Fonts->AddFontFromFileTTF() needs a file path.
    // Its parameters can also merge the font into the previous one, or load its color glyphs.
    //
    // Fonts are loaded at their nominal size: the scaling to the screen's DPI is applied at display
    // time by ImGui (ImGui::GetStyle().FontScaleDpi, set by the runner).

    //
    // Font loading parameters: several options are available (color, merging, range, ...)
    struct FontLoadingParams
    {
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


    // Loads a font (from the assets by default), with the loading parameters above
    ImFont* LoadFont(
        const std::string & fontFilename, float fontSize,
        const FontLoadingParams & params = {});

    // Loads a font from the assets, with an ImGui font config
    ImFont* LoadFontTTF(
        const std::string & fontFilename,
        float fontSize,
        ImFontConfig config = ImFontConfig()
    );

    // Loads a font from the assets and merges the icons of Font Awesome into it
    // (Font Awesome 4 or 6, as set by RunnerParams.callbacks.defaultIconFont)
    ImFont* LoadFontTTF_WithFontAwesomeIcons(
        const std::string & fontFilename,
        float fontSize,
        ImFontConfig configFont = ImFontConfig()
    );

    // ::endcode
}
