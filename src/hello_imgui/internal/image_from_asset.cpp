#include "hello_imgui/image_from_asset.h"

#include "hello_imgui/internal/image_abstract.h"
#include "hello_imgui/hello_imgui.h"
#include "image_opengl.h"
#include "image_dx11.h"
#include "image_metal.h"
#include "image_vulkan.h"

#include "hello_imgui/image_from_asset.h"
#include "hello_imgui/hello_imgui_assets.h"
#include "hello_imgui/runner_params.h"
#include "imgui.h"
#include "hello_imgui/hello_imgui_assets.h"
#include "hello_imgui/hello_imgui_logger.h"
#include "stb_image.h"

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

        HelloImGui::RendererBackendType rendererBackendType = HelloImGui::GetRunnerParams()->rendererBackendType;
        ImageAbstractPtr concreteImage;

        #ifdef HELLOIMGUI_HAS_OPENGL
            if (rendererBackendType == RendererBackendType::OpenGL3)
                concreteImage = std::make_shared<ImageOpenGl>();
        #endif
        #if defined(HELLOIMGUI_HAS_METAL)
            if (rendererBackendType == RendererBackendType::Metal)
                concreteImage = std::make_shared<ImageMetal>();
        #endif
        #if defined(HELLOIMGUI_HAS_VULKAN)
            if (rendererBackendType == RendererBackendType::Vulkan)
                concreteImage = std::make_shared<ImageVulkan>();
        #endif
        #if defined(HELLOIMGUI_HAS_DIRECTX11)
            if (rendererBackendType == RendererBackendType::DirectX11)
                concreteImage = std::make_shared<ImageDx11>();
        #endif
        if (concreteImage == nullptr)
            HelloImGui::Log(LogLevel::Warning, "ImageFromAsset: not implemented for this rendering backend!");
        gImageFromAssetMap[assetPath] = concreteImage;

        if (concreteImage != nullptr)
        {
            // Load the image using stbi_load_from_memory
            auto assetData = LoadAssetFileData(assetPath);
            IM_ASSERT(assetData.data != nullptr);
            unsigned char*image_data_rgba = stbi_load_from_memory(
                (unsigned char *)assetData.data, (int)assetData.dataSize,
                &concreteImage->Width, &concreteImage->Height, NULL, 4);
            if (image_data_rgba == NULL)
            {
                IM_ASSERT(false && "ImageDx11: Failed to load image!");
                throw std::runtime_error("ImageAbstract: Failed to load image!");
            }

            concreteImage->_impl_StoreTexture(concreteImage->Width, concreteImage->Height, image_data_rgba);
        }

        return gImageFromAssetMap.at(assetPath);
    }



    void ImageFromAsset(
        const char *assetPath, const ImVec2& size,
        const ImVec2& uv0, const ImVec2& uv1,
        const ImVec4& tint_col, const ImVec4& border_col)
    {
        auto cachedImage = _GetCachedImage(assetPath);
        if (cachedImage == nullptr)
        {
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "ImageFromAsset: fail!");
            return;
        }
        auto textureId = cachedImage->TextureID();
        auto imageSize = ImVec2((float)cachedImage->Width, (float)cachedImage->Height);
        ImVec2 displayedSize = ImageProportionalSize(size, imageSize);
        ImGui::Image(textureId, displayedSize, uv0, uv1, tint_col, border_col);
    }

    bool ImageButtonFromAsset(const char *assetPath, const ImVec2& size, const ImVec2& uv0,  const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
    {
        auto cachedImage = _GetCachedImage(assetPath);
        if (cachedImage == nullptr)
        {
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "ImageButtonFromAsset: fail!");
            return false;
        }
        auto textureId = cachedImage->TextureID();
        auto imageSize = ImVec2((float)cachedImage->Width, (float)cachedImage->Height);
        ImVec2 displayedSize = ImageProportionalSize(size, imageSize);
        bool clicked = ImGui::ImageButton(textureId, displayedSize, uv0, uv1, frame_padding, bg_col, tint_col);
        return clicked;
    }

    ImTextureID ImTextureIdFromAsset(const char *assetPath)
    {
        auto cachedImage = _GetCachedImage(assetPath);
        if (cachedImage == nullptr)
            return ImTextureID(0);
        return cachedImage->TextureID();
    }

    ImVec2 ImageSizeFromAsset(const char *assetPath)
    {
        auto cachedImage = _GetCachedImage(assetPath);
        if (cachedImage == nullptr)
            return ImVec2(0.f, 0.f);
        return ImVec2((float)cachedImage->Width, (float)cachedImage->Height);
    }

    ImageAndSize ImageAndSizeFromAsset(const char *assetPath)
    {
        auto cachedImage = _GetCachedImage(assetPath);
        if (cachedImage == nullptr)
            return {};
        return {cachedImage->TextureID(), ImVec2((float)cachedImage->Width, (float)cachedImage->Height)};
    }

    namespace internal
    {
        void Free_ImageFromAssetMap()
        {
            gImageFromAssetMap.clear();
        }
    }

}
