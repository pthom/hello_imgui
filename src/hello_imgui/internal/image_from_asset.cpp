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


struct SvgRgbaImage
{
    std::vector<unsigned char> data;
    int width;
    int height;
};

static SvgRgbaImage priv_SvgToRgba(
    const HelloImGui::AssetFileData& data,
    ImVec2 size = ImVec2(0.f, 0.f) // Use intrinsic width if 0, Use intrinsic height if 0. Preserve aspect ratio if only one is provided
);


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

    static std::string priv_FilenameAndSizeKey(const char* assetPath, ImVec2 size)
    {
        std::string key = assetPath;
        key += "_";
        key += std::to_string(size.x);
        key += "_";
        key += std::to_string(size.y);
        return key;
    }

    static bool priv_IsFilenameSvg(const std::string& filename)
    {
        return filename.size() > 4 && filename.substr(filename.size() - 4) == ".svg";
    }

    static std::unordered_map<std::string, ImageAbstractPtr > gImageFromAssetMap;


    static ImageAbstractPtr _GetCachedImage(const char*assetPath, ImVec2 sizeForSvg)
    {
        std::string key = priv_FilenameAndSizeKey(assetPath, sizeForSvg);

        if (gImageFromAssetMap.find(key) != gImageFromAssetMap.end())
            return gImageFromAssetMap.at(key);

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
        {
            HelloImGui::Log(LogLevel::Warning, "ImageFromAsset: not implemented for this rendering backend!");
            gImageFromAssetMap[key] = nullptr; // Cache the failure
            return nullptr;
        }

        bool isSvg = priv_IsFilenameSvg(assetPath);

        if (! isSvg)
        {
            // Load with stbimage
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

            gImageFromAssetMap[key] = concreteImage;
            return concreteImage;
        }
        else
        {
            // Load SVG
            SvgRgbaImage svgRgbaImage;
            {
                // Load the image using stbi_load_from_memory
                auto assetData = LoadAssetFileData(assetPath);
                IM_ASSERT(assetData.data != nullptr);
                svgRgbaImage = priv_SvgToRgba(assetData, sizeForSvg);
                FreeAssetFileData(&assetData);
                concreteImage->Height = svgRgbaImage.height;
                concreteImage->Width = svgRgbaImage.width;
            }
            concreteImage->_impl_StoreTexture(concreteImage->Width, concreteImage->Height, svgRgbaImage.data.data());

            gImageFromAssetMap[key] = concreteImage;
            return concreteImage;
        }
    }



    void ImageFromAsset(
        const char *assetPath, const ImVec2& size,
        const ImVec2& uv0, const ImVec2& uv1,
        const ImVec4& tint_col, const ImVec4& border_col)
    {
        auto cachedImage = _GetCachedImage(assetPath, size);
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
        auto cachedImage = _GetCachedImage(assetPath, size);
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

    ImTextureID ImTextureIdFromAsset(const char *assetPath, ImVec2 svgSize)
    {
        auto cachedImage = _GetCachedImage(assetPath, svgSize);
        if (cachedImage == nullptr)
            return ImTextureID(0);
        return cachedImage->TextureID();
    }

    ImVec2 ImageSizeFromAsset(const char *assetPath, ImVec2 svgSize)
    {
        auto cachedImage = _GetCachedImage(assetPath, svgSize);
        if (cachedImage == nullptr)
            return ImVec2(0.f, 0.f);
        return ImVec2((float)cachedImage->Width, (float)cachedImage->Height);
    }

    ImageAndSize ImageAndSizeFromAsset(const char *assetPath, ImVec2 svgSize)
    {
        auto cachedImage = _GetCachedImage(assetPath, svgSize);
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


#include "plutosvg.h"

static SvgRgbaImage priv_SvgToRgba(
    const HelloImGui::AssetFileData& assetFileData,
    ImVec2 size // Use intrinsic width if 0, Use intrinsic height if 0. Preserve aspect ratio if only one is provided
)
{
    // Load SVG document
    plutosvg_document_t* svg =
        plutosvg_document_load_from_data((const char*)assetFileData.data, assetFileData.dataSize, 0, 0, NULL, NULL);
    if (!svg) {
        IM_ASSERT(false && "SvgToRgba: Failed to load SVG document.");
    }

    // Get intrinsic dimensions
    float intrinsic_width = plutosvg_document_get_width(svg);
    float intrinsic_height = plutosvg_document_get_height(svg);
    if (intrinsic_width == 0 || intrinsic_height == 0) {
        plutosvg_document_destroy(svg);
        IM_ASSERT(false && "SvgToRgba: SVG document has invalid dimensions.");
    }

    // Determine render dimensions
    float width = size.x;
    float height = size.y;
    float render_width = width;
    float render_height = height;

    if (width == 0.0f && height == 0.0f) {
        render_width = intrinsic_width;
        render_height = intrinsic_height;
    } else if (width == 0.0f) {
        // Calculate width to preserve aspect ratio
        render_width = (height / intrinsic_height) * intrinsic_width;
    } else if (height == 0.0f) {
        // Calculate height to preserve aspect ratio
        render_height = (width / intrinsic_width) * intrinsic_height;
    }

    // Create a surface with the determined dimensions
    plutovg_surface_t* surface = plutovg_surface_create(static_cast<int>(render_width), static_cast<int>(render_height));
    if (!surface) {
        plutosvg_document_destroy(svg);
        IM_ASSERT(false && "SvgToRgba: Failed to create Plutovg surface.");
    }

    // Create a canvas for drawing
    plutovg_canvas_t* canvas = plutovg_canvas_create(surface);
    if (!canvas) {
        plutovg_surface_destroy(surface);
        plutosvg_document_destroy(svg);
        IM_ASSERT(false && "SvgToRgba: Failed to create Plutovg canvas.");
    }

    // Clear the surface with transparent color
    plutovg_color_t transparent = PLUTOVG_MAKE_COLOR(1, 1, 1, 0);
    //plutovg_surface_clear(surface, &transparent);

    // Set up palette callback (optional, can be nullptr if not using CSS variables)
    // For simplicity, we'll use nullptr here.
    bool render_success = plutosvg_document_render(svg, nullptr, canvas, &transparent, nullptr, nullptr);
    if (!render_success) {
        plutovg_canvas_destroy(canvas);
        plutovg_surface_destroy(surface);
        plutosvg_document_destroy(svg);
        IM_ASSERT(false && "SvgToRgba: Failed to render SVG document.");
    }

    // Access pixel data
    unsigned char* pixel_data = plutovg_surface_get_data(surface);
    if (!pixel_data) {
        plutovg_canvas_destroy(canvas);
        plutovg_surface_destroy(surface);
        plutosvg_document_destroy(svg);
        IM_ASSERT(false && "SvgToRgba: Failed to access surface pixel data.");
    }

    int stride = plutovg_surface_get_stride(surface);
    int height_px = plutovg_surface_get_height(surface);
    int width_px = plutovg_surface_get_width(surface);
    IM_ASSERT(stride == width_px * 4 && "SvgToRgba: Unexpected stride.");

    // Convert ARGB Premultiplied to RGBA Plain
    // Plutovg uses premultiplied ARGB, but we need plain RGBA for ImGui
    std::vector<unsigned  char> rgba_data(width_px * height_px * 4);
    plutovg_convert_argb_to_rgba(rgba_data.data(), pixel_data, width_px, height_px, stride);

    // Cleanup
    plutovg_canvas_destroy(canvas);
    plutovg_surface_destroy(surface);
    plutosvg_document_destroy(svg);

    return SvgRgbaImage{rgba_data, width_px, height_px};
}
