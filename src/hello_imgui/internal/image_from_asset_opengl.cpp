#ifdef HELLOIMGUI_HAS_OPENGL
#include "image_from_asset.h"

#include "hello_imgui/internal/image_opengl.h"
#include "hello_imgui/image_from_asset.h"
#include "imgui.h"
#include "hello_imgui/internal/stb_image.h"
#include "hello_imgui/hello_imgui_assets.h"

#include <string>
#include <unordered_map>

namespace HelloImGui
{
    static std::unordered_map<std::string, ImageOpenGlPtr> gImageFromAssetMap;


    static ImageOpenGlPtr _GetCachedImageMetal(const char*assetPath)
    {
        if (gImageFromAssetMap.find(assetPath) != gImageFromAssetMap.end())
            return gImageFromAssetMap.at(assetPath);

        // Load the image using stbi_load_from_memory
        auto assetData = LoadAssetFileData(assetPath);
        assert(assetData.data != nullptr);
        int width, height;
        unsigned char*image_data_rgba = stbi_load_from_memory(
            (unsigned char *)assetData.data, (int)assetData.dataSize,
            &width, &height, NULL, 4);
        if (image_data_rgba == NULL)
        {
            IM_ASSERT(false && "ImageMetal: Failed to load image!");
            return nullptr;
        }

        // Create and store the VkImage
        if (gImageFromAssetMap.find(assetPath) == gImageFromAssetMap.end())
            gImageFromAssetMap[assetPath] = std::make_shared<ImageOpenGl>(width, height, image_data_rgba);

        // Release image memory using stb
        stbi_image_free(image_data_rgba);

        return gImageFromAssetMap.at(assetPath);
    }

    void ImageFromAsset(
        const char *assetPath, const ImVec2& size,
        const ImVec2& uv0, const ImVec2& uv1,
        const ImVec4& tint_col, const ImVec4& border_col)
    {
        auto textureId = ImTextureIdFromAsset(assetPath);
        auto imageSize = ImageSizeFromAsset(assetPath);
        ImVec2 displayedSize = ImageProportionalSize(size, imageSize);
        ImGui::Image(textureId, displayedSize, uv0, uv1, tint_col, border_col);
    }

    bool ImageButtonFromAsset(const char *assetPath, const ImVec2& size, const ImVec2& uv0,  const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
    {
        auto textureId = ImTextureIdFromAsset(assetPath);
        auto imageSize = ImageSizeFromAsset(assetPath);
        ImVec2 displayedSize = ImageProportionalSize(size, imageSize);
        bool clicked = ImGui::ImageButton(textureId, displayedSize, uv0, uv1, frame_padding, bg_col, tint_col);
        return clicked;
    }

    ImTextureID ImTextureIdFromAsset(const char *assetPath)
    {
        auto cachedImage = _GetCachedImageMetal(assetPath);
        return (ImTextureID)(uintptr_t) cachedImage->TextureId;
    }

    ImVec2 ImageSizeFromAsset(const char *assetPath)
    {
        auto cachedImage = _GetCachedImageMetal(assetPath);
        return ImVec2((float)cachedImage->Width, (float)cachedImage->Height);
    }

    namespace internal
    {
        void Free_ImageFromAssetMap()
        {
            gImageFromAssetMap.clear();
        }
    }

} // namespace HelloImGui


#endif // #ifdef HELLOIMGUI_HAS_OPENGL
