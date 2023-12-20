#ifdef HELLOIMGUI_HAS_METAL
#ifdef HELLOIMGUI_USE_SDL2
#include "rendering_metal.h"

#import <Metal/Metal.h>
#include <backends/imgui_impl_metal.h>
#include <array>

#include "hello_imgui/hello_imgui.h"

#include <SDL.h>
#include <backends/imgui_impl_sdl2.h>


namespace HelloImGui
{
    SdlMetalGlobals& GetSdlMetalGlobals()
    {
        static SdlMetalGlobals sSdlMetalGlobals;
        return sSdlMetalGlobals;
    }


    void PrepareSdlForMetal_WithWindow_PreImGuiInit(SDL_Window* sdlWindow)
    {
        auto& gMetalGlobals = GetMetalGlobals();
        auto& gSdlMetalGlobals = GetSdlMetalGlobals();
        gSdlMetalGlobals.sdlWindow = sdlWindow;
        gSdlMetalGlobals.sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (gSdlMetalGlobals.sdlRenderer == nullptr)
        {
            bool Error_SdlCreateRenderer_For_Metal = false;
            IM_ASSERT(Error_SdlCreateRenderer_For_Metal);
            exit(-3);
        }

        // Setup Platform/Renderer backends
        gMetalGlobals.caMetalLayer = (__bridge CAMetalLayer*)SDL_RenderGetMetalLayer(gSdlMetalGlobals.sdlRenderer);
        gMetalGlobals.caMetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    }

    void PrepareSdlForMetal_PosImGuiInit()
    {
        auto& gMetalGlobals = GetMetalGlobals();
        auto& gSdlMetalGlobals = GetSdlMetalGlobals();

        ImGui_ImplMetal_Init(gMetalGlobals.caMetalLayer.device);
        ImGui_ImplSDL2_InitForMetal(gSdlMetalGlobals.sdlWindow);

        gMetalGlobals.mtlCommandQueue = [gMetalGlobals.caMetalLayer.device newCommandQueue];
        gMetalGlobals.mtlRenderPassDescriptor = [MTLRenderPassDescriptor new];
    }

    void SwapSdlMetalBuffers()
    {
        auto& gMetalGlobals = GetMetalGlobals();
        [gMetalGlobals.mtlRenderCommandEncoder popDebugGroup];
        [gMetalGlobals.mtlRenderCommandEncoder endEncoding];

        [gMetalGlobals.mtlCommandBuffer presentDrawable:gMetalGlobals.caMetalDrawable];
        [gMetalGlobals.mtlCommandBuffer commit];
    }

    RenderingCallbacks CreateBackendCallbacks_SdlMetal()
    {
        RenderingCallbacks callbacks;

        callbacks.Impl_NewFrame_3D = []
        {
            auto& gMetalGlobals = GetMetalGlobals();
            auto& gSdlMetalGlobals = GetSdlMetalGlobals();

            auto Vec4_To_Array = [](ImVec4 v) { return std::array<float, 4>{ v.x, v.y, v.z, v.w }; };

            //
            // New Frame + Clear color
            //
            int width, height;
            SDL_GetRendererOutputSize(gSdlMetalGlobals.sdlRenderer, &width, &height);
            gMetalGlobals.caMetalLayer.drawableSize = CGSizeMake(width, height);
            gMetalGlobals.caMetalDrawable = [gMetalGlobals.caMetalLayer nextDrawable];

            gMetalGlobals.mtlCommandBuffer = [gMetalGlobals.mtlCommandQueue commandBuffer];
            auto clearColor = Vec4_To_Array(HelloImGui::GetRunnerParams()->imGuiWindowParams.backgroundColor);
            gMetalGlobals.mtlRenderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clearColor[0] * clearColor[3], clearColor[1] * clearColor[3], clearColor[2] * clearColor[3], clearColor[3]);
            gMetalGlobals.mtlRenderPassDescriptor.colorAttachments[0].texture = gMetalGlobals.caMetalDrawable.texture;
            gMetalGlobals.mtlRenderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
            gMetalGlobals.mtlRenderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
            gMetalGlobals.mtlRenderCommandEncoder = [gMetalGlobals.mtlCommandBuffer renderCommandEncoderWithDescriptor:gMetalGlobals.mtlRenderPassDescriptor];
            [gMetalGlobals.mtlRenderCommandEncoder pushDebugGroup:@"ImGui demo"];

            // Start the Dear ImGui frame
            ImGui_ImplMetal_NewFrame(gMetalGlobals.mtlRenderPassDescriptor);
        };

        callbacks.Impl_RenderDrawData_To_3D = []
        {
            auto& gMetalGlobals = GetMetalGlobals();
            ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), gMetalGlobals.mtlCommandBuffer, gMetalGlobals.mtlRenderCommandEncoder);
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

} // namespace HelloImGui

#endif // HELLOIMGUI_USE_SDL2
#endif // HELLOIMGUI_HAS_METAL