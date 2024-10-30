#pragma once
#include "imgui.h"

namespace HelloImGui
{
// @@md#HelloImGui::ImageFromAsset

//
//Images are loaded when first displayed, and then cached
// (they will be freed just before the application exits).
//
//For example, given this files structure:
//```
//├── CMakeLists.txt
//├── assets/
//│         └── my_image.jpg
//└── my_app.main.cpp
//```
//
//then, you can display "my_image.jpg", using:
//
//    ```cpp
//    HelloImGui::ImageFromAsset("my_image.jpg");
//    ```


// `HelloImGui::ImageFromAsset(const char *assetPath, size, ...)`: 
// will display a static image from the assets.
void ImageFromAsset(const char *assetPath, const ImVec2& size = ImVec2(0, 0),
                    const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1,1),
                    const ImVec4& tint_col = ImVec4(1,1,1,1),
                    const ImVec4& border_col = ImVec4(0,0,0,0));

// `bool HelloImGui::ImageButtonFromAsset(const char *assetPath, size, ...)`:
// will display a button using an image from the assets.
bool ImageButtonFromAsset(const char *assetPath, const ImVec2& size = ImVec2(0, 0),
                          const ImVec2& uv0 = ImVec2(0, 0),  const ImVec2& uv1 = ImVec2(1,1),
                          int frame_padding = -1,
                          const ImVec4& bg_col = ImVec4(0,0,0,0),
                          const ImVec4& tint_col = ImVec4(1,1,1,1));

// `ImTextureID HelloImGui::ImTextureIdFromAsset(assetPath)`:
// will return a texture ID for an image loaded from the assets.
// The size param is used for SVG images, otherwise it is ignored:
// leave empty to use default SVG image size, otherwise if only one dimension is provided,
// the other will be computed to keep the aspect ratio.
ImTextureID ImTextureIdFromAsset(const char *assetPath, ImVec2 svgSize = ImVec2(0, 0));

// `ImVec2 HelloImGui::ImageSizeFromAsset(assetPath)`:
// will return the size of an image loaded from the assets.
// The size param is used for SVG images, otherwise it is ignored:
// leave empty to use default SVG image size, otherwise if only one dimension is provided,
// the other will be computed to keep the aspect ratio.
ImVec2 ImageSizeFromAsset(const char *assetPath, ImVec2 svgSize = ImVec2(0, 0));


// `HelloImGui::ImageAndSize HelloImGui::ImageAndSizeFromAsset(assetPath)`:
// will return the texture ID and the size of an image loaded from the assets.
// The size param is used for SVG images, otherwise it is ignored:
// leave empty to use default SVG image size, otherwise if only one dimension is provided,
// the other will be computed to keep the aspect ratio.
struct ImageAndSize
{
    ImTextureID textureId = ImTextureID(0);
    ImVec2 size = ImVec2(0.f, 0.f);
};
ImageAndSize ImageAndSizeFromAsset(const char *assetPath, ImVec2 svgSize = ImVec2(0, 0));


// `ImVec2 HelloImGui::ImageProportionalSize(askedSize, imageSize)`:
//  will return the displayed size of an image.
//     - if askedSize.x or askedSize.y is 0, then the corresponding dimension
//       will be computed from the image size, keeping the aspect ratio.
//     - if askedSize.x>0 and askedSize.y> 0, then the image will be scaled to fit
//       exactly the askedSize, thus potentially changing the aspect ratio.
//  Note: this function is used internally by ImageFromAsset and ImageButtonFromAsset,
//        so you don't need to call it directly.
ImVec2 ImageProportionalSize(const ImVec2& askedSize, const ImVec2& imageSize);

// @@md

namespace internal
{
    // Frees the internal texture cache
    void Free_ImageFromAssetMap();
}
}
