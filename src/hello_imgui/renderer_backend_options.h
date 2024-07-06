#pragma once
#include <string>
#include <optional>

namespace HelloImGui
{
// --------------------------------------------------------------------------------------------------------------------

// @@md#OpenGlOptions

// OpenGlOptions contains advanced options used at the startup of OpenGL.
// These parameters are reserved for advanced users.
// By default, Hello ImGui will select reasonable default values, and these parameters are not used.
// Use at your own risk, as they make break the multi-platform compatibility of your application!
// All these parameters are platform dependent.
// For real multiplatform examples, see
//     hello_imgui/src/hello_imgui/internal/backend_impls/opengl_setup_helper/opengl_setup_glfw.cpp
// and
//     hello_imgui/src/hello_imgui/internal/backend_impls/opengl_setup_helper/opengl_setup_sdl.cpp
//
// How to set those values manually:
// ---------------------------------
// you may set them manually:
//    (1) Either by setting them programmatically in your application
//        (set their values in `runnerParams.rendererBackendOptions.openGlOptions`)
//    (2) Either by setting them in a `hello_imgui.ini` file in the current folder, or any of its parent folders.
//       (this is useful when you want to set them for a specific app or set of apps, without modifying the app code)
//       See hello_imgui/hello_imgui_example.ini for an example of such a file.
// Note: if several methods are used, the order of priority is (1) > (2)
//
struct OpenGlOptions
{
    // Could be for example:
    //    "150" on macOS
    //    "130" on Windows
    //    "300es" on GLES
    std::string  GlslVersion = "130";

    // OpenGL 3.3 (these options won't work for GlEs)
    int          MajorVersion = 3;
    int          MinorVersion = 3;

    // OpenGL Core Profile (i.e. only includes the newer, maintained features of OpenGL)
    bool         UseCoreProfile = true;
    // OpenGL Forward Compatibility (required on macOS)
    bool         UseForwardCompat = true;

    // `AntiAliasingSamples`
    // If > 0, this value will be used to set the number of samples used for anti-aliasing.
    // This is used only when running with Glfw  + OpenGL (which is the default)
    // Notes:
    // - we query the maximum number of samples supported by the hardware, via glGetIntegerv(GL_MAX_SAMPLES)
    // - if you set this value to a non-zero value, it will be used instead of the default value of 8
    //   (except if it is greater than the maximum supported value, in which case a warning will be issued)
    // - if you set this value to 0, anti-aliasing will be disabled
    //
    // AntiAliasingSamples has a strong impact on the quality of the text rendering
    //     - 0: no anti-aliasing
    //     - 8: optimal
    //     - 16: optimal if using imgui-node-editor and you want to render very small text when unzooming
    int AntiAliasingSamples =  -1;  // -1 <=> not set (will use the default value of 8)
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