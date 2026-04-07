#pragma once
#include "imgui.h"

#include <memory>

namespace HelloImGui
{
// @@md#TextureGpu

// `HelloImGui::TextureGpu`: an opaque RAII handle that owns a GPU texture.
//
// `TextureGpu` is the abstract base class for backend-specific GPU texture
// objects (OpenGL, Metal, Vulkan, DirectX11). The concrete subclasses live
// inside `internal/` and are created by the factory functions below.
//
// Lifetime: the GPU resource is freed when the last `std::shared_ptr<TextureGpu>`
// reference is dropped — there is no separate `DeleteTexture()` to call.
//
// Threading: a live rendering backend (GL/Metal/Vulkan/Dx11) must exist on
// the current thread when constructing or destroying a `TextureGpu`. In
// practice this means: only call the factories from the GUI thread, while
// the application is running (i.e. between `HelloImGui::Run` startup and
// shutdown). Holding a `TextureGpuPtr` across the application's lifetime
// boundary is undefined behavior.
class TextureGpu
{
public:
    int Width = 0;
    int Height = 0;
    virtual ImTextureID TextureID() = 0;

    TextureGpu() = default;
    virtual ~TextureGpu();

    // Backend-specific upload of an HxWx4 RGBA buffer. Internal: prefer
    // calling the public factory `CreateTextureGpuFromRgbaData` instead.
    virtual void _impl_StoreTexture(int width, int height, unsigned char* image_data_rgba) = 0;
};

using TextureGpuPtr = std::shared_ptr<TextureGpu>;


// `HelloImGui::CreateTextureGpuFromRgbaData(rgbaData, width, height)`:
// Upload raw RGBA pixel data to the GPU and return an owning handle.
//
// - `rgbaData` must point to `width * height * 4` bytes of 8-bit RGBA pixels
//   (one byte per channel, row-major, no padding). The caller retains
//   ownership of `rgbaData`; the buffer is read once during the upload and
//   not referenced afterwards.
// - The returned `TextureGpuPtr` owns the GPU resource: when its last
//   reference drops, the texture is freed automatically.
// - Returns a null `TextureGpuPtr` if the rendering backend is not
//   supported or no live backend is available.
//
// Threading: must be called from the GUI thread, while a live rendering
// backend is initialized. Calling it before `HelloImGui::Run` has set up
// the backend (or after teardown) is undefined behavior.
TextureGpuPtr CreateTextureGpuFromRgbaData(
    const unsigned char* rgbaData, int width, int height);

// @@md
}
