#include "hello_imgui/hello_imgui_font.h"
#include "hello_imgui/dpi_aware.h"
#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/hello_imgui_assets.h"
#include "hello_imgui/hello_imgui_error.h"
#include "hello_imgui/icons_font_awesome_4.h"

#ifdef IMGUI_ENABLE_FREETYPE
#include "imgui_freetype.h"
#endif

#include <vector>

#ifdef IOS
#include "hello_imgui/internal/platform/getAppleBundleResourcePath.h"
#endif



namespace ImGui_SensibleFont
{
/*
    // Extract from imgui.h doc:
    // Common pitfalls:
    // 1. If you pass a 'glyph_ranges' array to AddFont*** functions, you need to make sure that your array persist up until the
    //   atlas is build (when calling GetTexData*** or Build()). We only copy the pointer, not the data.
    // 2. Important: By default, AddFontFromMemoryTTF() takes ownership of the data. Even though we are not writing to it, we will free the pointer on destruction.
    //   You can set font_cfg->FontDataOwnedByAtlas=false to keep ownership of your data, and it won't be freed,
    // 3. Even though many functions are suffixed with "TTF", OTF data is supported just as well.
    // This is an old API, and it is currently awkward for those and various other reasons! We will address them in the future!

    The functions below address the first pitfall, by storing the glyph_ranges in a static vector.
    It also tries to address the second pitfall, by adding an overload AddFontFromMemoryTTF_KeepOwnership, so that the pitfall can be seen by looking at the declarations.
    (Note: AddFontFromMemoryCompressedTTF and AddFontFromMemoryCompressedBase85TTF do not have this pitfall)

    They
    // IMGUI_API ImFont*           AddFontFromFileTTF(const char* filename, float size_pixels, const ImFontConfig* font_cfg = NULL, const ImWchar* glyph_ranges = NULL);
    // IMGUI_API ImFont*           AddFontFromMemoryTTF(void* font_data, int font_data_size, float size_pixels, const ImFontConfig* font_cfg = NULL, const ImWchar* glyph_ranges = NULL); // Note: Transfer ownership of 'ttf_data' to ImFontAtlas! Will be deleted after destruction of the atlas. Set font_cfg->FontDataOwnedByAtlas=false to keep ownership of your data and it won't be freed.

*/

    static void StoreStaticGlyphRange(ImFontConfig* font_cfg, const ImVector<ImWchar> & glyph_ranges = {})
    {
        // *Static* storage for pointers that will be used by ImGui::GetIO().Fonts->AddFontFromMemoryTTF, and which are required to persist until the Font Atlas is built, which happens much later after calling AddFontFromXXX
        //
        //                  *Open question: should the storage use "static" or "thread_local"?*
        //
        static ImVector<ImVector<ImWchar>> all_glyph_ranges; // This is kept alive, until the app exits. However, it is small.
        constexpr int max_size = 100;  // Arbitrary limit: if you load more than 100 fonts, you may want to increase this number
        bool first_time = all_glyph_ranges.Capacity == 0;

        if (first_time)
        {
            // Make sure that our static vector will not be moved in memory, unless the user load more than 100 fonts
            all_glyph_ranges.reserve(max_size);
        }

        // Add font config to static storage
        if (glyph_ranges.empty())
            font_cfg->GlyphRanges = nullptr;
        else
        {
            all_glyph_ranges.push_back({});
            ImVector<ImWchar> & glyph_ranges_static = all_glyph_ranges.back();
            glyph_ranges_static = glyph_ranges;
            font_cfg->GlyphRanges = glyph_ranges_static.Data;
        }

        IM_ASSERT(all_glyph_ranges.Size <= max_size && "Too many fonts loaded");
    }


    ImFont* AddFontFromFileTTF_2(const char* filename, float font_size_pixels, ImFontConfig* font_cfg = NULL, const ImVector<ImWchar> & glyph_ranges = {})
    {
        if (font_cfg != NULL && !glyph_ranges.empty())
            StoreStaticGlyphRange(font_cfg, glyph_ranges);
        return ImGui::GetIO().Fonts->AddFontFromFileTTF(filename, font_size_pixels, font_cfg);
    }

    ImFont* AddFontFromMemoryTTF_2(void* font_data, int font_data_size, float font_size_pixels, ImFontConfig* font_cfg = NULL, const ImVector<ImWchar> & glyph_ranges = {})
    {
        if (font_cfg != NULL && !glyph_ranges.empty())
            StoreStaticGlyphRange(font_cfg, glyph_ranges);
        return ImGui::GetIO().Fonts->AddFontFromMemoryTTF(font_data, font_data_size, font_size_pixels, font_cfg);
    }

    ImFont* AddFontFromMemoryTTF_2_KeepOwnership(void* font_data, int font_data_size, float font_size_pixels, ImFontConfig* font_cfg = NULL, const ImVector<ImWchar> & glyph_ranges = {})
    {
        if (font_cfg != NULL && !glyph_ranges.empty())
            StoreStaticGlyphRange(font_cfg, glyph_ranges);
        font_cfg->FontDataOwnedByAtlas = false;
        return ImGui::GetIO().Fonts->AddFontFromMemoryTTF(font_data, font_data_size, font_size_pixels, font_cfg);
    }
} // namespace ImGui_SensibleFont



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

    ImFont* _LoadFontImpl(const std::string & fontFilename, float fontSize_, const FontLoadingParams& params_)
    {
        gDidCallHelloImGuiLoadFontTTF = true;

        FontLoadingParams params = params_;

        // Font oversampling (set by dpiAwareParams)
        {
            const auto& dpiAwareParams = HelloImGui::GetDpiAwareParams();
            if (dpiAwareParams->fontOversampleH > 0)
                params.fontConfig.OversampleH = dpiAwareParams->fontOversampleH;
            if (dpiAwareParams->fontOversampleV > 0)
                params.fontConfig.OversampleV = dpiAwareParams->fontOversampleV;
        }

        float fontSize = fontSize_;
        if (params.adjustSizeToDpi)
            fontSize *= HelloImGui::DpiFontLoadingFactor();

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

        // Populate the glyph ranges for ImGui:
        // 2 value per range, values are inclusive, zero-terminated list
        ImVector<ImWchar> glyphRangesImVector;
        if (! params.glyphRanges.empty())
        {
            for (const auto & glyph_range_interval : params.glyphRanges)
            {
                glyphRangesImVector.push_back(glyph_range_interval[0]);
                glyphRangesImVector.push_back(glyph_range_interval[1]);
            }
            glyphRangesImVector.push_back(0); // Zero-terminate the array
        }

        if (params.insideAssets)
        {
            AssetFileData fontData = LoadAssetFileData(fontFilename.c_str());
            font = ImGui_SensibleFont::AddFontFromMemoryTTF_2_KeepOwnership(
                fontData.data, (int)fontData.dataSize, fontSize, &params.fontConfig, glyphRangesImVector);
            FreeAssetFileData(&fontData);
        }
        else
        {
            font = ImGui_SensibleFont::AddFontFromFileTTF_2(
                fontFilename.c_str(), fontSize, &params.fontConfig, glyphRangesImVector);
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

	std::vector<FontDpiResponsive> gAllDpiResponsiveFonts;
	bool gWasLoadFontBareCalled = false;
	bool gWasLoadFontDpiResponsiveCalled = false;


	ImFont* LoadFont(const std::string & fontFilename, float fontSize_, const FontLoadingParams& params_)
	{
		IM_ASSERT((!gWasLoadFontDpiResponsiveCalled) && "If using LoadFontDpiResponsive(), only use it, and do not use LoadFont()!");

        if (IsUsingHelloImGui())
        {
            auto runnerParams = HelloImGui::GetRunnerParams();
            IM_ASSERT(! runnerParams->dpiAwareParams.onlyUseFontDpiResponsive && "If runnerParams->dpiAwareParams.onlyUseFontDpiResponsive is true, you must use LoadFontDpiResponsive() instead of LoadFont()");
        }

		gWasLoadFontBareCalled = true;
		//printf("LoadFont(%s, %f)\n", fontFilename.c_str(), fontSize_);
		return _LoadFontImpl(fontFilename, fontSize_, params_);
	}

	FontDpiResponsive* LoadFontDpiResponsive(const std::string & fontFilename, float fontSize,
											const FontLoadingParams & fontLoadingParams)
	{
		IM_ASSERT((!gWasLoadFontBareCalled) && "If using LoadFontDpiResponsive(), set runnerParams.dpiAwareParams.onlyUseFontDpiResponsive=true and do not not use LoadFont()!");
		gWasLoadFontDpiResponsiveCalled = true;

		// Ensure that we have enough capacity, so that pointers remain valid
		constexpr size_t maxFonts = 100;
		if (gAllDpiResponsiveFonts.capacity() == 0)
			gAllDpiResponsiveFonts.reserve(maxFonts);
		// Whine if we are about to exceed the capacity
		IM_ASSERT(gAllDpiResponsiveFonts.size() < maxFonts - 1 && "Too many fonts loaded");
		// Insert a new element at the end of the vector
		gAllDpiResponsiveFonts.push_back({ nullptr, fontFilename, fontSize, fontLoadingParams });

		// Get the pointer to the newly inserted element (which we will return)
		FontDpiResponsive* dpiResponsiveFont = &gAllDpiResponsiveFonts.back();

		//printf("LoadFontDpiResponsive(%s, %f)\n", fontFilename.c_str(), fontSize);
		dpiResponsiveFont->font = _LoadFontImpl(fontFilename, fontSize, fontLoadingParams);
		dpiResponsiveFont->fontSize = fontSize;
		dpiResponsiveFont->fontFilename = fontFilename;
		dpiResponsiveFont->fontLoadingParams = fontLoadingParams;
		return dpiResponsiveFont;
	}

	bool _reloadAllDpiResponsiveFonts()
	{
		if (gWasLoadFontBareCalled)
		{
			fprintf(stderr, "_reloadAllDpiResponsiveFonts failed: ony call LoadFontDpiResponsive if you want this to work\n");
			return false;
		}
		if (gAllDpiResponsiveFonts.empty())
			return false;
		printf("_reloadAllDpiResponsiveFonts\n");
		auto& imguiFonts = ImGui::GetIO().Fonts;
		imguiFonts->Clear();
		for (auto & dpiResponsiveFont : gAllDpiResponsiveFonts)
		{
			float fontSize = dpiResponsiveFont.fontSize;
			const std::string & fontFilename = dpiResponsiveFont.fontFilename;
			const FontLoadingParams & fontLoadingParams = dpiResponsiveFont.fontLoadingParams;
			ImFont* newFont = _LoadFontImpl(fontFilename, fontSize, fontLoadingParams);
			dpiResponsiveFont.font = newFont;
		}
		bool buildSuccess = imguiFonts->Build();
		IM_ASSERT(buildSuccess && "_reloadAllDpiResponsiveFonts: Failed to build fonts");
		return true;
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

    std::vector<ImWcharPair> TranslateCommonGlyphRanges(const ImWchar* glyphRanges)
    {
        std::vector<ImWcharPair> glyphRangesPairs;
        size_t idx = 0;
        while (glyphRanges[idx] != 0)
        {
            ImWcharPair glyphRangePair = {glyphRanges[idx], glyphRanges[idx + 1]};
            glyphRangesPairs.push_back(glyphRangePair);
            idx += 2;
        }
        return glyphRangesPairs;
    }

    std::vector<ImWcharPair> translate_common_glyph_ranges(const std::vector<ImWchar> & glyphRanges)
    {
        std::vector<ImWcharPair> glyphRangesPairs;
        for (size_t i = 0; i < glyphRanges.size(); i += 2)
        {
            ImWcharPair glyphRangePair = { glyphRanges[i], glyphRanges[i + 1] };
            glyphRangesPairs.push_back(glyphRangePair);
        }
        return glyphRangesPairs;
    }

}