#pragma once
#include "imgui.h"
#include <string>

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
                    const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1,1));

// `HelloImGui::ImageFromAsset(const char *assetPath, size, ...)`:
// will display a static image from the assets, with a colored background and a border.
void ImageFromAssetWithBg(const char *assetPath, const ImVec2& size = ImVec2(0, 0),
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
ImTextureID ImTextureIdFromAsset(const char *assetPath);

// `ImVec2 HelloImGui::ImageSizeFromAsset(assetPath)`:
// will return the size of an image loaded from the assets.
ImVec2 ImageSizeFromAsset(const char *assetPath);


// `HelloImGui::ImageAndSize HelloImGui::ImageAndSizeFromAsset(assetPath)`:
// will return the texture ID and the size of an image loaded from the assets.
struct ImageAndSize
{
    ImTextureID textureId = ImTextureID(0);
    ImVec2 size = ImVec2(0.f, 0.f);
};
ImageAndSize ImageAndSizeFromAsset(const char *assetPath);


// `ImVec2 HelloImGui::ImageProportionalSize(askedSize, imageSize)`:
//  will return the displayed size of an image.
//     - if askedSize.x or askedSize.y is 0, then the corresponding dimension
//       will be computed from the image size, keeping the aspect ratio.
//     - if askedSize.x>0 and askedSize.y> 0, then the image will be scaled to fit
//       exactly the askedSize, thus potentially changing the aspect ratio.
//  Note: this function is used internally by ImageFromAsset and ImageButtonFromAsset,
//        so you don't need to call it directly.
ImVec2 ImageProportionalSize(const ImVec2& askedSize, const ImVec2& imageSize);

// `HelloImGui::ImageData`: decoded image pixel data (C++ only)
struct ImageData
{
    unsigned char* data = nullptr;
    int width = 0, height = 0;
    int channels = 0;
    void Free();
};

// `HelloImGui::LoadImageDataFromAsset(assetPath, desired_channels)`:
// (C++ only)
// Load and decode an image from the assets into CPU memory.
// desired_channels: 0=keep original, 1=gray, 3=RGB, 4=RGBA (default).
// The caller must call .free() on the returned ImageData when done!
ImageData LoadImageDataFromAsset(const char* assetPath, int desired_channels = 4);

// `HelloImGui::LoadImageDataFromEncodedData(data, dataSize, desired_channels)`:
// (C++ only)
// Load and decode encoded image data (PNG, JPEG, BMP, GIF, etc.) into CPU memory.
// desired_channels: 0=keep original, 1=gray, 3=RGB, 4=RGBA (default).
// The caller must call .free() on the returned ImageData when done!
// The image format is auto-detected from the data header by stb_image.
ImageData LoadImageDataFromEncodedData(const void* data, size_t dataSize, int desired_channels = 4);

// `HelloImGui::ImageAndSizeFromEncodedData(data, dataSize, cacheKey)`:
// will create a texture from encoded image data (PNG, JPEG, BMP, GIF, etc.),
// and return the texture ID and the size.
// - data: pointer to encoded image data
// - dataSize: size of the data in bytes
// - cacheKey: if non-empty, the texture will be cached and reused
//   on subsequent calls with the same key.
// The image format is auto-detected from the data header by stb_image.
ImageAndSize ImageAndSizeFromEncodedData(
    const void* data, size_t dataSize,
    const std::string& cacheKey = "");

// To upload raw RGBA pixel data to a caller-owned GPU texture, see
// `HelloImGui::CreateTextureGpuFromRgbaData()` in `texture_gpu.h`.

// @@md

namespace internal
{
    void Free_ImageFromAssetMap();
}
}
