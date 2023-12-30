#include "image_from_asset.h"


#if !defined(HELLOIMGUI_HAS_VULKAN) && !defined(HELLOIMGUI_HAS_OPENGL) && !defined(HELLOIMGUI_HAS_METAL)

#include "imgui.h"

namespace HelloImGui
{
    void ImageFromAsset(const char *assetPath, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.f, 0.f, 1.f));
        ImGui::Text("ImageFromAsset not available");
        ImGui::PopStyleColor();
    }

    bool ImageButtonFromAsset(const char *assetPath, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.f, 0.f, 1.f));
        ImGui::Text("ButtonFromAsset not available");
        ImGui::PopStyleColor();
        return false;
    }

    ImTextureID ImTextureIDFromAsset(const char *assetPath)
    {
        // Requires OpenGL!
        return nullptr;
    }

    ImVec2 ImageSizeFromAsset(const char *assetPath)
    {
        return ImVec2(0.f, 0.f);
    }

    namespace internal
    {
        void Free_ImageFromAssetMap() {}
    }
} // namespace HelloImGui

#endif
