#pragma once
#include <string>
#include <optional>

namespace HelloImGui
{
// --------------------------------------------------------------------------------------------------------------------

// @@md#OpenGlOptions

// OpenGlCallbacks contains advanced callbacks used at the startup of OpenGL.
// These parameters are reserved for advanced users.
// By default, Hello ImGui will select reasonable default values, and these parameters are not used.
// Use at your own risk, as they make break the multi-platform compatibility of your application!
// All these parameters are platform dependent.
// For real multiplatform examples, see
//     hello_imgui/src/hello_imgui/internal/backend_impls/opengl_setup_helper/opengl_setup_glfw.cpp
// and
//     hello_imgui/src/hello_imgui/internal/backend_impls/opengl_setup_helper/opengl_setup_sdl.cpp
struct OpenGlOptions
{
    // Could be for example:
    //    #version 150 on macOS
    //    #version 130 on Windows
    //    #version 300es on GLES
    std::string  GlslVersion = "#version 130";

    // OpenGL 3.3 (these options won't work for GlEs)
    int          MajorVersion = 3;
    int          MinorVersion = 3;

    // OpenGL Core Profile (i.e. only includes the newer, maintained features of OpenGL)
    bool         UseCoreProfile = true;
    // OpenGL Forward Compatibility (required on macOS)
    bool         UseForwardCompat = true;

    //    bool         UseGlEs = false;
    //    int          GlEsMajorVersion = 3;
};

// @@md



// @@md#RendererBackendOptions

// `bool hasEdrSupport()`:
// Check whether extended dynamic range (EDR), i.e. the ability to reproduce
// intensities exceeding the standard dynamic range from 0.0-1.0, is supported.
//
// To leverage EDR support, you need to set `floatBuffer=true` in `RendererBackendOptions`.
// Only the macOS Metal backend currently supports this.
//
// This currently returns false on all backends except Metal, where it checks whether
// this is supported on the current displays.
bool hasEdrSupport();


// RendererBackendOptions is a struct that contains options for the renderer backend
// (Metal, Vulkan, DirectX, OpenGL)
struct RendererBackendOptions
{
    // `requestFloatBuffer`:
    // Set to true to request a floating-point framebuffer.
    // Only available on Metal, if your display supports it.
    // Before setting this to true, first check `hasEdrSupport()`
    bool requestFloatBuffer = false;

    // `openGlOptions`:
    // Advanced options for OpenGL. Use at your own risk.
    std::optional<OpenGlOptions> openGlOptions = std::nullopt;
};


// Note:
// If using Metal, Vulkan or DirectX, you can find interesting pointers inside:
//     src/hello_imgui/internal/backend_impls/rendering_metal.h
//     src/hello_imgui/internal/backend_impls/rendering_vulkan.h
//     src/hello_imgui/internal/backend_impls/rendering_dx11.h
//     src/hello_imgui/internal/backend_impls/rendering_dx12.h

// @@md

}  // namespace HelloImGui