#ifdef HELLOIMGUI_HAS_METAL
#ifdef HELLOIMGUI_USE_SDL2
#include "rendering_metal.h"

#import <Metal/Metal.h>
#include "imgui_impl_metal.h"
#include <array>

#include "hello_imgui/hello_imgui.h"

#include <SDL.h>
#include "imgui_impl_sdl2.h"

#include <tuple>


namespace HelloImGui
{
    SdlMetalGlobals& GetSdlMetalGlobals()
    {
        static SdlMetalGlobals sSdlMetalGlobals;
        return sSdlMetalGlobals;
    }

    // Below is implementation of RenderingCallbacks_LinkWindowingToRenderingBackend
    void PrepareSdlForMetal(SDL_Window* sdlWindow, const RendererBackendOptions& rendererBackendOptions)
    {
        auto& gMetalGlobals = GetMetalGlobals();
        auto& gSdlMetalGlobals = GetSdlMetalGlobals();

        {
            gSdlMetalGlobals.sdlWindow = sdlWindow;
            gMetalGlobals.mtlDevice = MTLCreateSystemDefaultDevice();
            gSdlMetalGlobals.sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (gSdlMetalGlobals.sdlRenderer == nullptr)
            {
                bool Error_SdlCreateRenderer_For_Metal = false;
                IM_ASSERT(Error_SdlCreateRenderer_For_Metal);
                exit(-3);
            }

            // Setup Platform/Renderer backends
            gMetalGlobals.caMetalLayer = (__bridge CAMetalLayer*)SDL_RenderGetMetalLayer(gSdlMetalGlobals.sdlRenderer);

            if (rendererBackendOptions.requestFloatBuffer)
            {
                gMetalGlobals.caMetalLayer.pixelFormat = MTLPixelFormatRGBA16Float;
                gMetalGlobals.caMetalLayer.wantsExtendedDynamicRangeContent = YES;
                gMetalGlobals.caMetalLayer.colorspace = CGColorSpaceCreateWithName(kCGColorSpaceExtendedSRGB);
            }
            else
                gMetalGlobals.caMetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;

        }
        {
            ImGui_ImplMetal_Init(gMetalGlobals.caMetalLayer.device);
            ImGui_ImplSDL2_InitForMetal(gSdlMetalGlobals.sdlWindow);

            gMetalGlobals.mtlCommandQueue = [gMetalGlobals.caMetalLayer.device newCommandQueue];
            gMetalGlobals.mtlRenderPassDescriptor = [MTLRenderPassDescriptor new];
        }
    }


    RenderingCallbacksPtr CreateBackendCallbacks_SdlMetal()
    {
        auto callbacks = PrepareBackendCallbacksCommon();

        callbacks->Impl_GetFrameBufferSize = []
        {
            int width, height;
            SDL_GetRendererOutputSize(GetSdlMetalGlobals().sdlRenderer, &width, &height);
            return ScreenSize{width, height};
        };

        return callbacks;
    }

} // namespace HelloImGui

#endif // HELLOIMGUI_USE_SDL2
#endif // HELLOIMGUI_HAS_METAL