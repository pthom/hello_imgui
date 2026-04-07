#include "hello_imgui/image_from_asset.h"

#include "hello_imgui/texture_gpu.h"
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

    static std::unordered_map<std::string, TextureGpuPtr > gImageFromAssetMap;


    static TextureGpuPtr _GetCachedImage(const char*assetPath)
    {
        if (gImageFromAssetMap.find(assetPath) != gImageFromAssetMap.end())
            return gImageFromAssetMap.at(assetPath);

        HelloImGui::RendererBackendType rendererBackendType = HelloImGui::GetRunnerParams()->rendererBackendType;
        TextureGpuPtr concreteImage;

        #ifdef HELLOIMGUI_HAS_OPENGL
            if (rendererBackendType == RendererBackendType::OpenGL3)
                concreteImage = std::make_shared<TextureGpuOpenGl>();
        #endif
        #if defined(HELLOIMGUI_HAS_METAL)
            if (rendererBackendType == RendererBackendType::Metal)
                concreteImage = std::make_shared<TextureGpuMetal>();
        #endif
        #if defined(HELLOIMGUI_HAS_VULKAN)
            if (rendererBackendType == RendererBackendType::Vulkan)
                concreteImage = std::make_shared<TextureGpuVulkan>();
        #endif
        #if defined(HELLOIMGUI_HAS_DIRECTX11)
            if (rendererBackendType == RendererBackendType::DirectX11)
                concreteImage = std::make_shared<TextureGpuDx11>();
        #endif
        if (concreteImage == nullptr)
        {
            HelloImGui::Log(LogLevel::Warning, "ImageFromAsset: not implemented for this rendering backend!");
            gImageFromAssetMap[assetPath] = nullptr; // Cache the failure
            return nullptr;
        }

        unsigned char* image_data_rgba;
        {
            // Load the image using stbi_load_from_memory
            auto assetData = LoadAssetFileData(assetPath);
            IM_ASSERT(assetData.data != nullptr);
            image_data_rgba = stbi_load_from_memory(
                (unsigned char *)assetData.data, (int)assetData.dataSize,
                &concreteImage->Width, &concreteImage->Height, NULL, 4);
            FreeAssetFileData(&assetData);
        }

        if (image_data_rgba == NULL)
        {
            IM_ASSERT(false && "_GetCachedImage: Failed to load image!");
            throw std::runtime_error("_GetCachedImage: Failed to load image!");
        }
        concreteImage->_impl_StoreTexture(concreteImage->Width, concreteImage->Height, image_data_rgba);
        stbi_image_free(image_data_rgba);

        gImageFromAssetMap[assetPath] = concreteImage;
        return concreteImage;
    }


    void ImageFromAsset_Impl(
        const char *assetPath, const ImVec2& size,
        const ImVec2& uv0, const ImVec2& uv1,
        bool withBg,
        const ImVec4& tint_col = ImVec4(1,1,1,1),
        const ImVec4& border_col = ImVec4(0,0,0,0)
        )
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
        if (withBg)
            ImGui::ImageWithBg(textureId, displayedSize, uv0, uv1, tint_col, border_col);
        else
            ImGui::Image(textureId, displayedSize, uv0, uv1);
    }

    void ImageFromAsset(
        const char *assetPath, const ImVec2& size,
        const ImVec2& uv0, const ImVec2& uv1)
    {
        ImageFromAsset_Impl(assetPath, size, uv0, uv1, false);
    }

    void ImageFromAssetWithBg(
        const char *assetPath, const ImVec2& size,
        const ImVec2& uv0, const ImVec2& uv1,
        const ImVec4& tint_col, const ImVec4& border_col)
    {
        ImageFromAsset_Impl(assetPath, size, uv0, uv1, true, tint_col, border_col);
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
        bool clicked = ImGui::ImageButton(assetPath, textureId, displayedSize, uv0, uv1, bg_col, tint_col);
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

    void ImageData::Free()
    {
        if (data)
        {
            stbi_image_free(data);
            data = nullptr;
        }
        width = height = channels = 0;
    }

    ImageData LoadImageDataFromAsset(const char* assetPath, int desired_channels)
    {
        ImageData result;
        auto assetData = LoadAssetFileData(assetPath);
        if (assetData.data == nullptr)
        {
            HelloImGui::Log(LogLevel::Warning, "LoadImageDataFromAsset: failed to load asset %s", assetPath);
            return result;
        }
        result.data = stbi_load_from_memory(
            (unsigned char*)assetData.data, (int)assetData.dataSize,
            &result.width, &result.height, &result.channels, desired_channels);
        FreeAssetFileData(&assetData);
        if (result.data == nullptr)
        {
            HelloImGui::Log(LogLevel::Warning, "LoadImageDataFromAsset: failed to decode image %s", assetPath);
            return result;
        }
        if (desired_channels > 0)
            result.channels = desired_channels;
        return result;
    }

    // Create a backend-specific TextureGpu and upload RGBA data to GPU.
    // Does NOT free rgba_data - the caller is responsible for freeing it.
    static TextureGpuPtr _CreateTextureGpuFromRgba(unsigned char* rgba_data, int width, int height)
    {
        if (rgba_data == nullptr)
            return nullptr;

        HelloImGui::RendererBackendType rendererBackendType = HelloImGui::GetRunnerParams()->rendererBackendType;
        TextureGpuPtr concreteImage;

        #ifdef HELLOIMGUI_HAS_OPENGL
            if (rendererBackendType == RendererBackendType::OpenGL3)
                concreteImage = std::make_shared<TextureGpuOpenGl>();
        #endif
        #if defined(HELLOIMGUI_HAS_METAL)
            if (rendererBackendType == RendererBackendType::Metal)
                concreteImage = std::make_shared<TextureGpuMetal>();
        #endif
        #if defined(HELLOIMGUI_HAS_VULKAN)
            if (rendererBackendType == RendererBackendType::Vulkan)
                concreteImage = std::make_shared<TextureGpuVulkan>();
        #endif
        #if defined(HELLOIMGUI_HAS_DIRECTX11)
            if (rendererBackendType == RendererBackendType::DirectX11)
                concreteImage = std::make_shared<TextureGpuDx11>();
        #endif

        if (concreteImage == nullptr)
        {
            HelloImGui::Log(LogLevel::Warning, "_CreateTextureGpuFromRgba: not implemented for this rendering backend!");
            return nullptr;
        }

        concreteImage->Width = width;
        concreteImage->Height = height;
        concreteImage->_impl_StoreTexture(width, height, rgba_data);
        return concreteImage;
    }


    ImageData LoadImageDataFromEncodedData(const void* data, size_t dataSize, int desired_channels)
    {
        ImageData result;
        result.data = stbi_load_from_memory(
            (const unsigned char*)data, (int)dataSize,
            &result.width, &result.height, &result.channels, desired_channels);
        if (result.data == nullptr)
        {
            HelloImGui::Log(LogLevel::Warning, "LoadImageDataFromEncodedData: failed to decode image data");
            return result;
        }
        if (desired_channels > 0)
            result.channels = desired_channels;
        return result;
    }


    ImageAndSize ImageAndSizeFromEncodedData(
        const void* data, size_t dataSize,
        const std::string& cacheKey)
    {
        // Check cache
        if (!cacheKey.empty() && gImageFromAssetMap.find(cacheKey) != gImageFromAssetMap.end())
        {
            auto& cached = gImageFromAssetMap.at(cacheKey);
            if (cached)
                return {cached->TextureID(), ImVec2((float)cached->Width, (float)cached->Height)};
            return {};
        }

        // Decode with stbi
        int width, height;
        unsigned char* rgba_data = stbi_load_from_memory(
            (const unsigned char*)data, (int)dataSize,
            &width, &height, NULL, 4);

        if (rgba_data == NULL)
        {
            HelloImGui::Log(LogLevel::Warning, "ImageAndSizeFromEncodedData: failed to decode image data");
            return {};
        }

        auto concreteImage = _CreateTextureGpuFromRgba(rgba_data, width, height);
        stbi_image_free(rgba_data);
        if (!concreteImage)
            return {};

        // Cache if key provided
        if (!cacheKey.empty())
            gImageFromAssetMap[cacheKey] = concreteImage;

        return {concreteImage->TextureID(), ImVec2((float)width, (float)height)};
    }


    TextureGpuPtr CreateTextureGpuFromRgbaData(
        const unsigned char* rgbaData, int width, int height)
    {
        // _CreateTextureGpuFromRgba uploads the pixel data into a fresh
        // backend-specific TextureGpu and returns the owning shared_ptr.
        // We just forward it: callers own the lifetime.
        return _CreateTextureGpuFromRgba(
            const_cast<unsigned char*>(rgbaData), width, height);
    }

    namespace internal
    {
        void Free_ImageFromAssetMap()
        {
            gImageFromAssetMap.clear();
        }
    }

}
