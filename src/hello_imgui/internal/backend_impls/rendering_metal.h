#pragma once
#ifdef HELLOIMGUI_HAS_METAL

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

#ifdef HELLOIMGUI_USE_SDL2
struct SDL_Renderer;
struct SDL_Window;
#endif

#include "hello_imgui/internal/backend_impls/rendering_callbacks.h"

namespace HelloImGui
{
    RenderingCallbacks CreateBackendCallbacks_Metal();

#ifdef HELLOIMGUI_USE_SDL2
    void PrepareSdLForMetal_WithWindow_PreImGuiInit(SDL_Window* window);
    void PrepareSdLForMetal_PosImGuiInit();
    void SwapSdlMetalBuffers();

    struct SdlMetalGlobals
    {
        SDL_Window* sdlWindow = nullptr;
        SDL_Renderer* sdlRenderer = nullptr;
        CAMetalLayer* caMetalLayer = nullptr;
        id<CAMetalDrawable> caMetalDrawable = nullptr;
        id<MTLCommandBuffer> mtlCommandBuffer = nullptr;
        id<MTLCommandQueue> mtlCommandQueue = nullptr;
        MTLRenderPassDescriptor* mtlRenderPassDescriptor = nullptr;
        id <MTLRenderCommandEncoder> mtlRenderCommandEncoder = nullptr;
    };

    SdlMetalGlobals& GetSdlMetalGlobals();
#endif
}

#endif // HELLOIMGUI_HAS_METAL
