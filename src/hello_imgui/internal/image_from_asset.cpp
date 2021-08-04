#ifdef HELLOIMGUI_HAS_OPENGL
#include "hello_imgui/image_gl.h"

#include <string>
#include <unordered_map>
#include <exception>

namespace HelloImGui
{
std::unordered_map<std::string, ImageGlPtr> gImageFromAssetMap;

void _LoadImageGl(const char *assetPath)
{
    if (gImageFromAssetMap.find(assetPath) == gImageFromAssetMap.end())
        gImageFromAssetMap[assetPath] = ImageGl::FactorImage(assetPath);
}

void ImageFromAsset(const char *assetPath, const ImVec2 &size, const ImVec2 &uv0, const ImVec2 &uv1,
                    const ImVec4 &tint_col, const ImVec4 &border_col)
{
    _LoadImageGl(assetPath);
    gImageFromAssetMap.at(assetPath)->Draw(size, uv0, uv1, tint_col, border_col);
}

bool ImageButtonFromAsset(const char *assetPath, const ImVec2 &size, const ImVec2 &uv0, const ImVec2 &uv1,
                          int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col)
{
    _LoadImageGl(assetPath);
    return gImageFromAssetMap.at(assetPath)->DrawButton(size, uv0, uv1, frame_padding, bg_col, tint_col);
}

ImTextureID ImTextureIdFromAsset(const char *assetPath)
{
    _LoadImageGl(assetPath);
    return gImageFromAssetMap.at(assetPath)->imTextureId;
}


namespace internal
{
    void Free_ImageFromAssetMap()
    {
        // this function is called  by HelloImGui during the application's TearDown
        // and will clear all asset images textures when the OpenGL context is still valid.
        gImageFromAssetMap.clear();
    }
}

} // namespace HelloImGui

#else // #ifdef HELLOIMGUI_HAS_OPENGL

namespace HelloImGui
{
void ImageFromAsset(const char *assetPath, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
{
    throw std::runtime_error("ImageFromAsset is only available with OpenGL backends at this time, sorry!")
}

bool ImageButtonFromAsset(const char *assetPath, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
{
    throw std::runtime_error("ImageButtonFromAsset is only available with OpenGL backends at this time, sorry!")
}

ImTextureID ImTextureIDFromAsset(const char *assetPath)
{
    throw std::runtime_error("ImTextureIDFromAsset is only available with OpenGL backends at this time, sorry!")
}

namespace internal
{
    void Free_ImageFromAssetMap() {}
}
} // namespace HelloImGui
#endif // #ifdef HELLOIMGUI_HAS_OPENGL
