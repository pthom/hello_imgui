#include "image_from_asset.h"

#include "hello_imgui/internal/image_abstract.h"
#include "image_opengl.h"
#include "image_dx11.h"
#include "image_metal.h"
#include "image_vulkan.h"

#include "hello_imgui/image_from_asset.h"
#include "hello_imgui/hello_imgui_assets.h"
#include "imgui.h"
#include "hello_imgui/hello_imgui_assets.h"
#include "hello_imgui/internal/stb_image.h"

#include <string>
#include <unordered_map>
#include <stdexcept>


namespace HelloImGui
{
    ImVec2 ImageProportionalSize(const ImVec2& askedSize, const ImVec2& imageSize)
    {
        ImVec2 r(askedSize);

        if ((r.x == 0.f) && (r.y == 0.f))
            r = imageSize;
        else if (r.y == 0.f)
            r.y = imageSize.y / imageSize.x * r.x;
        else if (r.x == 0.f)
            r.x = imageSize.x / imageSize.y * r.y;
        return r;
    }

    static std::unordered_map<std::string, ImageAbstractPtr > gImageFromAssetMap;


    static ImageAbstractPtr _GetCachedImage(const char*assetPath)
    {
        if (gImageFromAssetMap.find(assetPath) != gImageFromAssetMap.end())
            return gImageFromAssetMap.at(assetPath);

        ImageAbstractPtr image;
#ifdef HELLOIMGUI_HAS_OPENGL
        image = std::make_shared<ImageOpenGl>();
#elif defined(HELLOIMGUI_HAS_DIRECTX11)
        image = std::make_shared<ImageDx11>();
#elif defined(HELLOIMGUI_HAS_METAL)
        image = std::make_shared<ImageMetal>();
#elif defined(HELLOIMGUI_HAS_VULKAN)
        image = std::make_shared<ImageVulkan>();
#else
#error "No graphics API defined"
#endif

        gImageFromAssetMap[assetPath] = image;

        // Load the image using stbi_load_from_memory
        auto assetData = LoadAssetFileData(assetPath);
        assert(assetData.data != nullptr);
        unsigned char*image_data_rgba = stbi_load_from_memory(
            (unsigned char *)assetData.data, (int)assetData.dataSize,
            &image->Width, &image->Height, NULL, 4);
        if (image_data_rgba == NULL)
        {
            IM_ASSERT(false && "ImageDx11: Failed to load image!");
            throw std::runtime_error("ImageAbstract: Failed to load image!");
        }

        image->_impl_StoreTexture(image->Width, image->Height, image_data_rgba);
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
        auto cachedImage = _GetCachedImage(assetPath);
        return cachedImage->TextureID();
    }

    ImVec2 ImageSizeFromAsset(const char *assetPath)
    {
        auto cachedImage = _GetCachedImage(assetPath);
        return ImVec2((float)cachedImage->Width, (float)cachedImage->Height);
    }

    namespace internal
    {
        void Free_ImageFromAssetMap()
        {
            gImageFromAssetMap.clear();
        }
    }

}
