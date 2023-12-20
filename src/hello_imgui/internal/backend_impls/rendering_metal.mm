#ifdef HELLOIMGUI_HAS_METAL
#include "rendering_metal.h"

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#include <backends/imgui_impl_metal.h>
#include <array>

#define STB_IMAGE_IMPLEMENTATION
#include "hello_imgui/internal/stb_image.h"
#include "hello_imgui/hello_imgui.h"

#ifdef HELLOIMGUI_USE_SDL2
#include <SDL.h>
#include <backends/imgui_impl_sdl2.h>
#endif


namespace HelloImGui
{
    #ifdef HELLOIMGUI_USE_SDL2

    SdlMetalGlobals gSdlMetalGlobals;

    void PrepareSdLForMetal_WithWindow_PreImGuiInit(SDL_Window* sdlWindow)
    {
        gSdlMetalGlobals.sdlWindow = sdlWindow;
        gSdlMetalGlobals.sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (gSdlMetalGlobals.sdlRenderer == nullptr)
        {
            bool Error_SdlCreateRenderer_For_Metal = false;
            IM_ASSERT(Error_SdlCreateRenderer_For_Metal);
            exit(-3);
        }

        // Setup Platform/Renderer backends
        gSdlMetalGlobals.caMetalLayer = (__bridge CAMetalLayer*)SDL_RenderGetMetalLayer(gSdlMetalGlobals.sdlRenderer);
        gSdlMetalGlobals.caMetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    }

    void PrepareSdLForMetal_PosImGuiInit()
    {
        ImGui_ImplMetal_Init(gSdlMetalGlobals.caMetalLayer.device);
        ImGui_ImplSDL2_InitForMetal(gSdlMetalGlobals.sdlWindow);

        gSdlMetalGlobals.mtlCommandQueue = [gSdlMetalGlobals.caMetalLayer.device newCommandQueue];
        gSdlMetalGlobals.mtlRenderPassDescriptor = [MTLRenderPassDescriptor new];
    }

    void SwapSdlMetalBuffers()
    {
        [gSdlMetalGlobals.mtlRenderCommandEncoder popDebugGroup];
        [gSdlMetalGlobals.mtlRenderCommandEncoder endEncoding];

        [gSdlMetalGlobals.mtlCommandBuffer presentDrawable:gSdlMetalGlobals.caMetalDrawable];
        [gSdlMetalGlobals.mtlCommandBuffer commit];
    }

    RenderingCallbacks CreateBackendCallbacks_Metal()
    {
        RenderingCallbacks callbacks;

        callbacks.Impl_NewFrame_3D = []
        {
            auto Vec4_To_Array = [](ImVec4 v) { return std::array<float, 4>{ v.x, v.y, v.z, v.w }; };

            //
            // New Frame + Clear color
            //
            int width, height;
            SDL_GetRendererOutputSize(gSdlMetalGlobals.sdlRenderer, &width, &height);
            gSdlMetalGlobals.caMetalLayer.drawableSize = CGSizeMake(width, height);
            gSdlMetalGlobals.caMetalDrawable = [gSdlMetalGlobals.caMetalLayer nextDrawable];

            gSdlMetalGlobals.mtlCommandBuffer = [gSdlMetalGlobals.mtlCommandQueue commandBuffer];
            auto clearColor = Vec4_To_Array(HelloImGui::GetRunnerParams()->imGuiWindowParams.backgroundColor);
            gSdlMetalGlobals.mtlRenderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clearColor[0] * clearColor[3], clearColor[1] * clearColor[3], clearColor[2] * clearColor[3], clearColor[3]);
            gSdlMetalGlobals.mtlRenderPassDescriptor.colorAttachments[0].texture = gSdlMetalGlobals.caMetalDrawable.texture;
            gSdlMetalGlobals.mtlRenderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
            gSdlMetalGlobals.mtlRenderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
            gSdlMetalGlobals.mtlRenderCommandEncoder = [gSdlMetalGlobals.mtlCommandBuffer renderCommandEncoderWithDescriptor:gSdlMetalGlobals.mtlRenderPassDescriptor];
            [gSdlMetalGlobals.mtlRenderCommandEncoder pushDebugGroup:@"ImGui demo"];

            // Start the Dear ImGui frame
            ImGui_ImplMetal_NewFrame(gSdlMetalGlobals.mtlRenderPassDescriptor);
        };

        callbacks.Impl_RenderDrawData_To_3D = []
        {
            ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), gSdlMetalGlobals.mtlCommandBuffer, gSdlMetalGlobals.mtlRenderCommandEncoder);
        };

        // Not implemented for Metal
        //callbacks.Impl_ScreenshotRgb = []() { ...;};

        // This is done at Impl_NewFrame_3D
        callbacks.Impl_Frame_3D_ClearColor = [](ImVec4 clearColor) { };

        callbacks.Impl_Shutdown_3D = []
        {
            ImGui_ImplMetal_Shutdown();
        };
        return callbacks;
    }

    SdlMetalGlobals& GetSdlMetalGlobals()
    {
        return gSdlMetalGlobals;
    }


#endif // #ifdef HELLOIMGUI_USE_SDL2


} // namespace HelloImGui

#endif // HELLOIMGUI_HAS_METAL