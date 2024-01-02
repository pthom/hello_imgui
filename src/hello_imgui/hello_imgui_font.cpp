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

    ImFont* AddFontFromFileTTF(
        const char* filename,
        float font_size,
        const ImFontConfig& font_cfg = ImFontConfig(),
        const ImVector<ImWchar[2]> & glyph_ranges = {}
    )
    {
        ImFontConfig* static_font_config = MakeStaticFontConfig(font_cfg, glyph_ranges);
        static_font_config->FontDataOwnedByAtlas = false;
        float font_size_pixels_adjusted = font_size * FontLoadingFactor();
        return ImGui::GetIO().Fonts->AddFontFromFileTTF(filename, font_size_pixels_adjusted, static_font_config);
    }

    ImFont* AddFontFromMemoryTTF_KeepOwnership(
        void* font_data,
        int font_data_size,
        float font_size,
        const ImFontConfig& font_cfg = ImFontConfig(),
        const ImVector<ImWchar[2]> & glyph_ranges = {}
    )
    {
        ImFontConfig* static_font_config = MakeStaticFontConfig(font_cfg, glyph_ranges);
        static_font_config->FontDataOwnedByAtlas = false;
        float font_size_pixels_adjusted = font_size * FontLoadingFactor();

        return ImGui::GetIO().Fonts->AddFontFromMemoryTTF(
            font_data, font_data_size, font_size_pixels_adjusted, static_font_config);
    }

    ImFont* AddFontFromMemoryTTF_TransferOwnerShip(
        void* font_data,
        int font_data_size,
        float font_size,
        const ImFontConfig& font_cfg = ImFontConfig(),
        const ImVector<ImWchar[2]> & glyph_ranges = {}
    )
    {
        ImFontConfig* static_font_config = MakeStaticFontConfig(font_cfg, glyph_ranges);
        static_font_config->FontDataOwnedByAtlas = true;
        float font_size_pixels_adjusted = font_size * FontLoadingFactor();
        return ImGui::GetIO().Fonts->AddFontFromMemoryTTF(
            font_data, font_data_size, font_size_pixels_adjusted, static_font_config);
    }
}


namespace HelloImGui
{
    bool gDidCallHelloImGuiLoadFontTTF = false;

    ImFont* LoadFontFromFileTTF(
        const std::string& filename,
        float fontSize,
        bool mergeToLastFont,
        const ImFontConfig& fontConfig,
        const ImVector<ImWchar[2]> & glyphRanges)
    {
        ImFontConfig font_cfg_copy = fontConfig;
        font_cfg_copy.MergeMode = mergeToLastFont;
        ImFont* font = ImGui_SensibleFont::AddFontFromFileTTF(filename.c_str(), fontSize, font_cfg_copy, glyphRanges);
        gDidCallHelloImGuiLoadFontTTF = true;
        return font;
    }

    ImFont* LoadFontFromAssetFileTTF(
        const std::string& filename,
        float fontSize,
        bool mergeToLastFont,
        const ImFontConfig& fontConfig,
        const ImVector<ImWchar[2]> & glyphRanges)
    {
        if (!HelloImGui::AssetExists(filename))
        {
            fprintf(stderr, "Cannot find font %s\n", filename.c_str());
            return nullptr;
        }
        AssetFileData fontData = LoadAssetFileData(filename.c_str());

        ImFontConfig font_cfg_copy = fontConfig;
        font_cfg_copy.MergeMode = mergeToLastFont;
        ImFont* font = ImGui_SensibleFont::AddFontFromMemoryTTF_KeepOwnership(
            fontData.data, (int)fontData.dataSize, fontSize, font_cfg_copy, glyphRanges);

        gDidCallHelloImGuiLoadFontTTF = true;
        FreeAssetFileData(&fontData);
        return font;
    }

    ImFont* LoadFontTTF(const std::string & fontFilename, float fontSize, bool useFullGlyphRange, ImFontConfig config)
    {
        ImVector<ImWchar[2]> glyph_ranges;
        if (useFullGlyphRange)
            glyph_ranges.push_back({0x0020, 0xFFFF});
        ImFont *font = LoadFontFromAssetFileTTF(fontFilename, fontSize, false, config, glyph_ranges);
        return font;
    }

    ImFont* LoadFontTTF_WithFontAwesomeIcons(
        const std::string & fontFilename,
        float fontSize,
        bool useFullGlyphRange,
        ImFontConfig configFont,
        ImFontConfig configIcons)
    {
        ImFont *font = LoadFontTTF(fontFilename, fontSize, useFullGlyphRange, configFont);
        if (font == nullptr)
            return nullptr;
        font = MergeFontAwesomeToLastFont(fontSize, configIcons);
        return font;
    }

    ImFont* MergeFontAwesomeToLastFont(float fontSize, ImFontConfig config)
    {
        static std::string faFile = "fonts/fontawesome-webfont.ttf";
        ImVector<ImWchar[2]> glyph_ranges;
        glyph_ranges.push_back({ ICON_MIN_FA, ICON_MAX_FA });
        ImFont* font = LoadFontFromAssetFileTTF(faFile, fontSize, true, config, glyph_ranges);
        return font;
    }


#ifdef IMGUI_ENABLE_FREETYPE
    ImFont* LoadEmojiFontFromAssetFileTTF(
        const std::string & fontFilename,
        float fontSize,
        bool mergeToLastFont,
        const ImVector<ImWchar[2]> & glyph_ranges)
    {
        static ImWchar fullGlyphRange[2] = { 0x1, 0x1FFFF };

        ImVector<ImWchar[2]> glyph_ranges_copy = glyph_ranges;
        if (glyph_ranges_copy.empty())
            glyph_ranges_copy.push_back(fullGlyphRange);

        auto createEmojiConfig = []() -> ImFontConfig {
            ImFontConfig cfg;
            cfg.OversampleH = cfg.OversampleV = 1;
            cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
            return cfg;
        };

        ImFontConfig emojiConfig = createEmojiConfig();
        ImFont *font = LoadFontFromAssetFileTTF(fontFilename, fontSize, mergeToLastFont, emojiConfig, glyph_ranges_copy);
        return font;
    }
    #endif


    bool DidCallHelloImGuiLoadFontTTF()
    {
        return HelloImGui::gDidCallHelloImGuiLoadFontTTF;
    }

}