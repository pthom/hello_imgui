#ifdef HELLOIMGUI_HAS_VULKAN
#include "hello_imgui/image_from_asset.h"
#include "imgui.h"
#include "hello_imgui/internal/backend_impls/image_vulkan.h"

#include <string>
#include <unordered_map>

namespace HelloImGui
{
    std::unordered_map<std::string, ImageVkPtr> gImageFromAssetMap;

    void ImageFromAsset(
        const char *assetPath, const ImVec2& size,
        const ImVec2& uv0, const ImVec2& uv1,
        const ImVec4& tint_col, const ImVec4& border_col)
    {
        auto textureId = ImTextureIdFromAsset(assetPath);

        ImGui::Image(textureId, size, uv0, uv1, tint_col, border_col);
    }

    bool ImageButtonFromAsset(const char *assetPath, const ImVec2& size, const ImVec2& uv0,  const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
    {
        auto textureId = ImTextureIdFromAsset(assetPath);
        bool clicked = ImGui::ImageButton(textureId, size, uv0, uv1, frame_padding, bg_col, tint_col);
        return clicked;
    }

    ImTextureID ImTextureIdFromAsset(const char *assetPath)
    {
        if (gImageFromAssetMap.find(assetPath) == gImageFromAssetMap.end())
                gImageFromAssetMap[assetPath] = std::make_shared<ImageVk>(assetPath);

        return (ImTextureID) gImageFromAssetMap.at(assetPath)->DS;
    }

    namespace internal
    {
        void Free_ImageFromAssetMap()
        {
            gImageFromAssetMap.clear();
        }
    }

} // namespace HelloImGui
#endif // #ifdef HELLOIMGUI_HAS_VULKAN
