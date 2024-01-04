#pragma once


namespace HelloImGui
{
/**
 @@md#RendererBackendOptions

**RendererBackendOptions** is a struct that contains options for the renderer backend (Metal, Vulkan, DirectX, ...).
 Members:
* `metalOptions`: _MetalOptions_. Options for the Metal backend (only filled if the Metal backend is available)


**MetalOptions** is a struct that contains options for the Metal backend.
 Members:
* `fixmeDummyOptionName`: _bool, default=false_. Dummy option for Metal backend (to be completed)


Note: If using the Metal, Vulkan or DirectX rendering backend, you can find some interesting pointers inside
 `src/hello_imgui/internal/backend_impls/rendering_metal.h`
 `src/hello_imgui/internal/backend_impls/rendering_vulkan.h`
 `src/hello_imgui/internal/backend_impls/rendering_dx11.h`
 `src/hello_imgui/internal/backend_impls/rendering_dx12.h`

@@md
*/


#ifdef HELLOIMGUI_HAS_METAL
struct MetalOptions
{
    // to be completed
    bool fixmeDummyOptionName = false;
};
#endif

struct RendererBackendOptions
{
#ifdef HELLOIMGUI_HAS_METAL
    MetalOptions metalOptions;
#endif
};

} // namespace HelloImGui