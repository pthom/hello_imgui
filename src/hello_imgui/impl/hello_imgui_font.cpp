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
#include <cstring>
#include <cmath>

#ifdef IOS
#include "hello_imgui/internal/platform/getAppleBundleResourcePath.h"
#endif


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

    // Fills ImFontConfig->Name with the base filename of the font file + the font size
    // (ImFontConfig->Name is only used for debugging purposes, and displayed in the Style Editor)
    static void Priv_CopyDebugFontNameToFontConfig(
        const std::string& fontFilename,
        float fontSize,
        ImFontConfig* dstFontConfig)
    {
        // Step 1: Extract the base filename without path
        std::string filename = fontFilename;
        size_t lastSlash = filename.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            filename = filename.substr(lastSlash + 1);
        }

        // Step 2: Remove the extension
        size_t lastDot = filename.find_last_of('.');
        if (lastDot != std::string::npos) {
            filename = filename.substr(0, lastDot);
        }

        // Step 3: Round fontSize to the nearest integer
        int roundedSize = static_cast<int>(std::lround(fontSize));

        // Step 4: Create the name string (e.g., "Arial 10")
        std::string nameWithSize = filename + " " + std::to_string(roundedSize);

        // Step 5: Copy the result into dstFontConfig->Name, ensuring no overflow
        int bufferSize = sizeof(dstFontConfig->Name);
        std::strncpy(dstFontConfig->Name, nameWithSize.c_str(), bufferSize - 1);
        dstFontConfig->Name[bufferSize - 1] = '\0'; // Ensure null termination
    }

    ImFont* _LoadFontImpl(const std::string & fontFilename, float fontSize_, const FontLoadingParams& params_)
    {
        gDidCallHelloImGuiLoadFontTTF = true;

        FontLoadingParams params = params_;

        Priv_CopyDebugFontNameToFontConfig(fontFilename, fontSize_, &params.fontConfig);

        float fontSize = fontSize_;
        if (params.adjustSizeToDpi)
            fontSize *= HelloImGui::DpiFontLoadingFactor();

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
            params.fontConfig.FontDataOwnedByAtlas = false;
            font = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(fontData.data, (int)fontData.dataSize, fontSize, &params.fontConfig);
            FreeAssetFileData(&fontData);
        }
        else
        {
            font = ImGui::GetIO().Fonts->AddFontFromFileTTF(fontFilename.c_str(), fontSize, &params.fontConfig);
        }

        if (params.mergeFontAwesome)
        {
            IM_ASSERT(params.insideAssets && "FontLoadingParmas.mergeFontAwesome requires params.insideAssets");
            static std::string faFile = "fonts/fontawesome-webfont.ttf";
            FontLoadingParams fontLoadingParamsFa;
            fontLoadingParamsFa.fontConfig = params.fontConfigFontAwesome;
            fontLoadingParamsFa.mergeToLastFont = true;
            fontLoadingParamsFa.adjustSizeToDpi = params.adjustSizeToDpi;
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
			// fprintf(stderr, "_reloadAllDpiResponsiveFonts failed: ony call LoadFontDpiResponsive if you want this to work\n");
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
        ImFont* font = LoadFont(faFile, fontSize, fontLoadingParams);
        return font;
    }


    bool DidCallHelloImGuiLoadFontTTF()
    {
        return HelloImGui::gDidCallHelloImGuiLoadFontTTF;
    }

}