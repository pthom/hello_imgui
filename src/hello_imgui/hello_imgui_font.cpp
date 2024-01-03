#include "hello_imgui_font.h"
#include "hello_imgui_assets.h"
#include "hello_imgui_error.h"
#include "hello_imgui.h"
#include "icons_font_awesome.h"
#include "dpi_aware.h"

#ifdef IMGUI_ENABLE_FREETYPE
#include "misc/freetype/imgui_freetype.h"
#endif



#ifdef IOS
#include "hello_imgui/internal/platform/getAppleBundleResourcePath.h"
#endif


// Patch ImGui font loading with sensible defaults
//    - Do not require static
namespace ImGui_SensibleFont
{
    static ImFontConfig* MakeStaticFontConfig(const ImFontConfig& font_cfg, const ImVector<ImWchar[2]> & glyph_ranges = {})
    {
        // Storage for pointers that will be used by ImGui::GetIO().Fonts->AddFontFromMemoryTTF
        // (and which are required to persist during the application lifetime)
        static ImVector<ImFontConfig> all_cfgs;
        static ImVector<ImVector<ImWchar>> all_glyph_ranges;
        bool first_time = all_cfgs.Capacity == 0;
        if (first_time)
        {
            // try to make sure that our static vector will not be moved in memory. Yeah, I know...
            // If you use more than 100 fonts, you may want to increase this number (...)
            all_cfgs.reserve(100);
            all_glyph_ranges.reserve(100);
        }

        // Add font config to static storage
        all_cfgs.push_back(font_cfg);
        ImFontConfig* font_cfg_static = &(all_cfgs.back());

        if (glyph_ranges.empty())
            font_cfg_static->GlyphRanges = nullptr;
        else
        {
            // Doc from imgui.h:
            //    const ImWchar*  GlyphRanges;            // NULL
            //    THE ARRAY DATA NEEDS TO PERSIST AS LONG AS THE FONT IS ALIVE.
            //    Pointer to a user-provided list of Unicode range
            //    (2 value per range, values are inclusive, zero-terminated list).
            // Add an empty vector for the glyph ranges
            all_glyph_ranges.push_back({});
            ImVector<ImWchar> & glyph_ranges_static = all_glyph_ranges.back();

            // Populate the glyph ranges
            for (const auto & glyph_range : glyph_ranges)
            {
                glyph_ranges_static.push_back(glyph_range[0]);
                glyph_ranges_static.push_back(glyph_range[1]);
            }
            glyph_ranges_static.push_back(0); // Zero-terminate the array

            font_cfg_static->GlyphRanges = glyph_ranges_static.Data;
        }

        return font_cfg_static;
    }

    ImFont* AddFontFromFileTTF(const char* filename, float font_size_pixels, const ImFontConfig& font_cfg = ImFontConfig(), const ImVector<ImWchar[2]> & glyph_ranges = {})
    {
        ImFontConfig* static_font_config = MakeStaticFontConfig(font_cfg, glyph_ranges);
        static_font_config->FontDataOwnedByAtlas = false;
        return ImGui::GetIO().Fonts->AddFontFromFileTTF(filename, font_size_pixels, static_font_config);
    }

    ImFont* AddFontFromMemoryTTF(void* font_data, int font_data_size, float font_size_pixels, const ImFontConfig& font_cfg = ImFontConfig(), const ImVector<ImWchar[2]> & glyph_ranges = {})
    {
        ImFontConfig* static_font_config = MakeStaticFontConfig(font_cfg, glyph_ranges);
        return ImGui::GetIO().Fonts->AddFontFromMemoryTTF(font_data, font_data_size, font_size_pixels, static_font_config);
    }

    ImFont* AddFontFromMemoryTTF_KeepOwnership(void* font_data, int font_data_size, float font_size_pixels, const ImFontConfig& font_cfg = ImFontConfig(), const ImVector<ImWchar[2]> & glyph_ranges = {})
    {
        ImFontConfig* static_font_config = MakeStaticFontConfig(font_cfg, glyph_ranges);
        static_font_config->FontDataOwnedByAtlas = false;
        return ImGui::GetIO().Fonts->AddFontFromMemoryTTF(font_data, font_data_size, font_size_pixels, static_font_config);
    }
}


namespace HelloImGui
{
    bool gDidCallHelloImGuiLoadFontTTF = false;

    float WindowContentScale()
    {
        /*
        // On SDL, this could be implemented as:
        float ddpi, hdpi, vdpi;
        int displayIndex = 0;
        if (SDL_GetDisplayDPI(displayIndex, &ddpi, &hdpi, &vdpi) != 0)
            return 1.f;
        // 96PPI is the "standard" DPI for a screen, we use it as a reference
        float targetPpi = 96.f;
        return ddpi / targetPpi;

        // On glfw, it could be:
        float xscale, yscale;
        glfwGetWindowContentScale((GLFWwindow *) window, &xscale, &yscale);
        if (xscale > yscale)
            return xscale;
        else
            return yscale;
        */
            return 1.f;
    }

    // A factor applied to font loading size, to account for HighDPI
    // so that rendering is comparable on all platforms, given a desired font size
    static float FontLoadingFactor()
    {
        // Possible implementation outside of HelloImGui:
        // float k = WindowContentScale() * 1.f / ImGui::GetIO().FontGlobalScale;
        // return k;

        return HelloImGui::DpiFontLoadingFactor();
    }



    ImFont* LoadFont(const std::string & fontFilename, float fontSize_, const FontLoadingParams& params_)
    {
        gDidCallHelloImGuiLoadFontTTF = true;

        FontLoadingParams params = params_;

        float fontSize = fontSize_;
        if (params.adjustSizeToDpi)
            fontSize *= FontLoadingFactor();

        if (params.useFullGlyphRange)
        {
            params.glyphRanges.clear();
#ifdef IMGUI_USE_WCHAR32
            params.glyphRanges.push_back({ 0x0001, 0x1FFFF });
#else
            params.glyphRanges.push_back({ 0x0001, 0xFFFF });
#endif
            if (params.reduceMemoryUsageIfFullGlyphRange)
                params.fontConfig.OversampleH = params.fontConfig.OversampleV = 1;
        }

        if (params.loadColor)
        {
#ifdef IMGUI_ENABLE_FREETYPE
            params.fontConfig.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
#else
            IM_ASSERT(false && "FontLoadingParmas.loadColor requires freetype (IMGUI_ENABLE_FREETYPE)");
            return nullptr;
#endif
        }

        params.fontConfig.MergeMode = params.mergeToLastFont;

        ImFont* font = nullptr;
        if (params.insideAssets)
        {
            AssetFileData fontData = LoadAssetFileData(fontFilename.c_str());
            font = ImGui_SensibleFont::AddFontFromMemoryTTF_KeepOwnership(
                fontData.data, fontData.dataSize, fontSize, params.fontConfig, params.glyphRanges);
            FreeAssetFileData(&fontData);
        }
        else
        {
            font = ImGui_SensibleFont::AddFontFromFileTTF(
                fontFilename.c_str(), fontSize, params.fontConfig, params.glyphRanges);
        }

        if (params.mergeFontAwesome)
        {
            IM_ASSERT(params.insideAssets && "FontLoadingParmas.mergeFontAwesome requires params.insideAssets");
            static std::string faFile = "fonts/fontawesome-webfont.ttf";
            FontLoadingParams fontLoadingParamsFa;
            fontLoadingParamsFa.fontConfig = params.fontConfigFontAwesome;
            fontLoadingParamsFa.mergeToLastFont = true;
            fontLoadingParamsFa.adjustSizeToDpi = params.adjustSizeToDpi;
            fontLoadingParamsFa.glyphRanges.push_back({ ICON_MIN_FA, ICON_MAX_FA });
            font = LoadFont(faFile, fontSize_, fontLoadingParamsFa);
        }

        return font;
    }

    ImFont* LoadFontTTF(const std::string & fontFilename, float fontSize, bool useFullGlyphRange, ImFontConfig config)
    {
        FontLoadingParams fontLoadingParams;
        if (useFullGlyphRange)
            fontLoadingParams.glyphRanges.push_back({0x0020, 0xFFFF});
        fontLoadingParams.fontConfig = config;
        ImFont* font = LoadFont(fontFilename, fontSize, fontLoadingParams);
        return font;
    }

    ImFont* LoadFontTTF_WithFontAwesomeIcons(
        const std::string & fontFilename,
        float fontSize,
        bool useFullGlyphRange,
        ImFontConfig configFont,
        ImFontConfig configIcons)
    {
        FontLoadingParams fontLoadingParams;
        if (useFullGlyphRange)
            fontLoadingParams.glyphRanges.push_back({0x0020, 0xFFFF});
        fontLoadingParams.fontConfig = configFont;
        fontLoadingParams.mergeFontAwesome = true;
        fontLoadingParams.fontConfigFontAwesome = configIcons;

        ImFont* font = LoadFont(fontFilename, fontSize, fontLoadingParams);
        return font;
    }

    ImFont* MergeFontAwesomeToLastFont(float fontSize, ImFontConfig config)
    {
        static std::string faFile = "fonts/fontawesome-webfont.ttf";

        FontLoadingParams fontLoadingParams;
        fontLoadingParams.mergeToLastFont = true;
        fontLoadingParams.fontConfig = config;
        fontLoadingParams.fontConfig.MergeMode = true;
        fontLoadingParams.glyphRanges.push_back({ ICON_MIN_FA, ICON_MAX_FA });

        ImFont* font = LoadFont(faFile, fontSize, fontLoadingParams);
        return font;
    }


    bool DidCallHelloImGuiLoadFontTTF()
    {
        return HelloImGui::gDidCallHelloImGuiLoadFontTTF;
    }

}