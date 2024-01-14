#pragma once
#ifdef HELLOIMGUI_HAS_METAL

#ifdef HELLOIMGUI_USE_SDL2
struct SDL_Renderer;
struct SDL_Window;
#endif

#ifdef HELLOIMGUI_USE_GLFW3
struct GLFWwindow;
#endif

#include "hello_imgui/internal/backend_impls/rendering_callbacks.h"
#include "hello_imgui/renderer_backend_options.h"

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

// For more info, see "Anatomy of an ImGui app lifecycle (cf ImGui examples)", in rendering_callbacks.h

namespace HelloImGui
{
    struct MetalGlobals
    {
        CAMetalLayer* caMetalLayer = nullptr;
        id<CAMetalDrawable> caMetalDrawable = nullptr;
        id<MTLCommandBuffer> mtlCommandBuffer = nullptr;
        id<MTLCommandQueue> mtlCommandQueue = nullptr;
        MTLRenderPassDescriptor* mtlRenderPassDescriptor = nullptr;
        id <MTLRenderCommandEncoder> mtlRenderCommandEncoder = nullptr;
        id<MTLDevice> mtlDevice = nullptr;
    };
    MetalGlobals& GetMetalGlobals();

    //  Impl of RenderingCallbacks_Impl_SwapBuffers
    void SwapMetalBuffers();

    RenderingCallbacksPtr PrepareBackendCallbacksCommon();

#ifdef HELLOIMGUI_USE_SDL2
    RenderingCallbacksPtr CreateBackendCallbacks_SdlMetal();

    void PrepareSdlForMetal(SDL_Window* window, const RendererBackendOptions& rendererBackendOptions);

    struct SdlMetalGlobals
    {
        SDL_Window* sdlWindow = nullptr;
        SDL_Renderer* sdlRenderer = nullptr;
    };
    SdlMetalGlobals& GetSdlMetalGlobals();
#endif

#ifdef HELLOIMGUI_USE_GLFW3
    RenderingCallbacksPtr CreateBackendCallbacks_GlfwMetal();

    void PrepareGlfwForMetal(GLFWwindow* window, const RendererBackendOptions& rendererBackendOptions);

    struct GlfwMetalGlobals
    {
        GLFWwindow* glfwWindow = nullptr;
    };
    GlfwMetalGlobals& GetGlfwMetalGlobals();
#endif

}

#endif // HELLOIMGUI_HAS_METAL
