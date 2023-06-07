#pragma once

#ifdef HELLOIMGUI_HAS_OPENGL
#include "hello_imgui/hello_imgui_assets.h"
#include "imgui.h"
#include <memory>

/**
@@md#HelloImGui::ImageGl

    Image loading utilities, based on stb_image. They work *only with OpenGL backends*.

    * `ImageGl::ImageGlPtr FactorImage(const char *assetPath)`: will load an image from an asset
     and return a unique_ptr<ImageGl>, which you can display with the Draw() or DrawButton() methods.
    *  `ImageGl::Draw(...)` and `ImageGl::DrawButton(...)` will draw the image. They have the same
     behavior as ImGui::Image (except that the size can be inferred from the loaded image size)

    _Note: Since ImageGl is not copiable, it has a private constructor; and you should use it via ImageGlPtr_
    ```cpp
    using ImageGlPtr = std::unique_ptr<ImageGl>;
    ```

@@md
*/
namespace HelloImGui
{
struct ImageGl;
using ImageGlPtr = std::unique_ptr<ImageGl>;

struct ImageGl
{
private:
    ImageGl(const char *assetPath);
    ImageGl(const char *assetPath, bool isGif);
public:
    static ImageGlPtr FactorImage(const char *assetPath);
    static ImageGlPtr FactorImage(const char *assetPath, bool isGif);
    ~ImageGl();

    void Draw(const ImVec2& size = ImVec2(0, 0), const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1,1), const ImVec4& tint_col = ImVec4(1,1,1,1), const ImVec4& border_col = ImVec4(0,0,0,0));
    void Draw(bool isGif, const ImVec2& size = ImVec2(0, 0), const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1,1), const ImVec4& tint_col = ImVec4(1,1,1,1), const ImVec4& border_col = ImVec4(0,0,0,0));
    bool DrawButton(const ImVec2& size = ImVec2(0, 0), const ImVec2& uv0 = ImVec2(0, 0),  const ImVec2& uv1 = ImVec2(1,1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0,0,0,0), const ImVec4& tint_col = ImVec4(1,1,1,1));    // <0 frame_padding uses default frame padding settings. 0 for no padding
    bool DrawButton(bool isGif, const ImVec2& size = ImVec2(0, 0), const ImVec2& uv0 = ImVec2(0, 0),  const ImVec2& uv1 = ImVec2(1,1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0,0,0,0), const ImVec4& tint_col = ImVec4(1,1,1,1));    // <0 frame_padding uses default frame padding settings. 0 for no padding

    ImVec2 imageSize;
    ImTextureID imTextureId;
    
    bool isGif = false;
    int Gif_Fps = 0;
    float Gif_Time = 0.0f; // playback progress
    ImTextureID* imTextureId_StaticImage = nullptr;
    ImTextureID* imTextureId_GifArray = nullptr;

};


}
#endif // #ifdef HELLOIMGUI_HAS_OPENGL
