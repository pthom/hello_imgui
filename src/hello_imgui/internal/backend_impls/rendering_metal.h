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

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>


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
    };
    MetalGlobals& GetMetalGlobals();

    void SwapMetalBuffers();
    RenderingCallbacksPtr PrepareBackendCallbacksCommon();

#ifdef HELLOIMGUI_USE_SDL2
    RenderingCallbacksPtr CreateBackendCallbacks_SdlMetal();

    void PrepareSdlForMetal_WithWindow_PreImGuiInit(SDL_Window* window);
    void PrepareSdlForMetal_PosImGuiInit();

    struct SdlMetalGlobals
    {
        SDL_Window* sdlWindow = nullptr;
        SDL_Renderer* sdlRenderer = nullptr;
    };
    SdlMetalGlobals& GetSdlMetalGlobals();
#endif

#ifdef HELLOIMGUI_USE_GLFW3
    RenderingCallbacksPtr CreateBackendCallbacks_GlfwMetal();

    void PrepareGlfwForMetal_WithWindow_PreImGuiInit(GLFWwindow* window);
    void PrepareGlfwForMetal_PosImGuiInit();

    struct GlfwMetalGlobals
    {
        GLFWwindow* glfwWindow = nullptr;
        id<MTLDevice> mtlDevice = nullptr;
    };
    GlfwMetalGlobals& GetGlfwMetalGlobals();
#endif

}

#endif // HELLOIMGUI_HAS_METAL
