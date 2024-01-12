#pragma once

namespace HelloImGui
{

/**
    Check whether extended dynamic range (EDR), i.e. the ability to reproduce intensities exceeding the
    standard dynamic range from 0.0-1.0, is supported.

    To leverage EDR support, you will need to set `floatBuffer=true` in `RendererBackendOptions`.
    Only the macOS Metal backend currently supports this.

    \return This currently returns false on all backends except Metal, where it checks whether this is
    supported on the current displays.
*/
bool hasEdrSupport();

/**
 @@md#RendererBackendOptions

**RendererBackendOptions** is a struct that contains options for the renderer backend (Metal, Vulkan, DirectX,
...). Members:
* `requestFloatBuffer`: _bool, default=false_. Set to true to request a floating-point framebuffer.
        Before setting this to true, first check `hasEdrSupport`


Note: If using the Metal, Vulkan or DirectX rendering backend, you can find some interesting pointers inside
 `src/hello_imgui/internal/backend_impls/rendering_metal.h`
 `src/hello_imgui/internal/backend_impls/rendering_vulkan.h`
 `src/hello_imgui/internal/backend_impls/rendering_dx11.h`
 `src/hello_imgui/internal/backend_impls/rendering_dx12.h`

@@md
*/

struct RendererBackendOptions
{

    bool requestFloatBuffer = false;
};

}  // namespace HelloImGui