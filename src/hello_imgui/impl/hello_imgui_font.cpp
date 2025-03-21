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

    ImFont* LoadFont(const std::string & fontFilename, float fontSize_, const FontLoadingParams& params_)
    {
        return _LoadFontImpl(fontFilename, fontSize_, params_);
    }


    bool DidCallHelloImGuiLoadFontTTF()
    {
        return HelloImGui::gDidCallHelloImGuiLoadFontTTF;
    }

}