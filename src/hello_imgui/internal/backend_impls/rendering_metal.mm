#ifdef HELLOIMGUI_HAS_METAL
#include "rendering_metal.h"

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#include <backends/imgui_impl_metal.h>
#include <array>

#define STB_IMAGE_IMPLEMENTATION
#include "hello_imgui/internal/stb_image.h"

#ifdef HELLOIMGUI_USE_SDL2
#include <SDL.h>
#include <backends/imgui_impl_sdl2.h>
#endif


namespace HelloImGui
{
    /*

    //
    // Init SDL
    //

    // Inform SDL that we will be using metal for rendering. Without this hint initialization of metal renderer may fail.
    X SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr)
    {
        printf("Error creating renderer: %s\n", SDL_GetError());
        return -3;
    }

    //
    // X Link SDL / Metal
    //

    // Setup Platform/Renderer backends
    CAMetalLayer* layer = (__bridge CAMetalLayer*)SDL_RenderGetMetalLayer(renderer);
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    ImGui_ImplMetal_Init(layer.device);
    ImGui_ImplSDL2_InitForMetal(window);

    id<MTLCommandQueue> commandQueue = [layer.device newCommandQueue];
    MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor new];

    //
    // X New Frame + Clear color
    //
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);
    layer.drawableSize = CGSizeMake(width, height);
    id<CAMetalDrawable> drawable = [layer nextDrawable];

    id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
    renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clear_color[0] * clear_color[3], clear_color[1] * clear_color[3], clear_color[2] * clear_color[3], clear_color[3]);
    renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
    renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
    [renderEncoder pushDebugGroup:@"ImGui demo"];
    // Start the Dear ImGui frame
    ImGui_ImplMetal_NewFrame(renderPassDescriptor);

    //
    // RenderDrawDataTo3D
    //
    ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), commandBuffer, renderEncoder);

    //
    // Post UpdatePlatformWindows + RenderPlatformWindowsDefault
    //
    [renderEncoder popDebugGroup];
    [renderEncoder endEncoding];

    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];

    //
    // ShutDown
    //
    X ImGui_ImplMetal_Shutdown();

    */

    #ifdef HELLOIMGUI_USE_SDL2


    struct SdlMetalGlobals
    {
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
        CAMetalLayer* layer = nullptr;
        id<CAMetalDrawable> drawable = nullptr;
        id<MTLCommandBuffer> commandBuffer = nullptr;
        id<MTLCommandQueue> commandQueue = nullptr;
        MTLRenderPassDescriptor* renderPassDescriptor = nullptr;
        id <MTLRenderCommandEncoder> renderEncoder = nullptr;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    };

    SdlMetalGlobals gSdlMetalGlobals;

    void PrepareSdLForMetal_WithWindow_PreImGuiInit(SDL_Window* window)
    {
        gSdlMetalGlobals.window = window;
        gSdlMetalGlobals.renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (gSdlMetalGlobals.renderer == nullptr)
        {
            bool Error_SdlCreateRenderer_For_Metal = false;
            IM_ASSERT(Error_SdlCreateRenderer_For_Metal);
            exit(-3);
        }

        // Setup Platform/Renderer backends
        gSdlMetalGlobals.layer = (__bridge CAMetalLayer*)SDL_RenderGetMetalLayer(gSdlMetalGlobals.renderer);
        gSdlMetalGlobals.layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    }

    void PrepareSdLForMetal_PosImGuiInit()
    {
        ImGui_ImplMetal_Init(gSdlMetalGlobals.layer.device);
        ImGui_ImplSDL2_InitForMetal(gSdlMetalGlobals.window);

        gSdlMetalGlobals.commandQueue = [gSdlMetalGlobals.layer.device newCommandQueue];
        gSdlMetalGlobals.renderPassDescriptor = [MTLRenderPassDescriptor new];
    }

    void SwapSdlMetalBuffers()
    {
        [gSdlMetalGlobals.renderEncoder popDebugGroup];
        [gSdlMetalGlobals.renderEncoder endEncoding];

        [gSdlMetalGlobals.commandBuffer presentDrawable:gSdlMetalGlobals.drawable];
        [gSdlMetalGlobals.commandBuffer commit];
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
            SDL_GetRendererOutputSize(gSdlMetalGlobals.renderer, &width, &height);
            gSdlMetalGlobals.layer.drawableSize = CGSizeMake(width, height);
            gSdlMetalGlobals.drawable = [gSdlMetalGlobals.layer nextDrawable];

            gSdlMetalGlobals.commandBuffer = [gSdlMetalGlobals.commandQueue commandBuffer];
            auto clear_color = Vec4_To_Array(gSdlMetalGlobals.clear_color);
            gSdlMetalGlobals.renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clear_color[0] * clear_color[3], clear_color[1] * clear_color[3], clear_color[2] * clear_color[3], clear_color[3]);
            gSdlMetalGlobals.renderPassDescriptor.colorAttachments[0].texture = gSdlMetalGlobals.drawable.texture;
            gSdlMetalGlobals.renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
            gSdlMetalGlobals.renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
            gSdlMetalGlobals.renderEncoder = [gSdlMetalGlobals.commandBuffer renderCommandEncoderWithDescriptor:gSdlMetalGlobals.renderPassDescriptor];
            [gSdlMetalGlobals.renderEncoder pushDebugGroup:@"ImGui demo"];

            // Start the Dear ImGui frame
            ImGui_ImplMetal_NewFrame(gSdlMetalGlobals.renderPassDescriptor);
        };

        callbacks.Impl_RenderDrawData_To_3D = []
        {
            ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), gSdlMetalGlobals.commandBuffer, gSdlMetalGlobals.renderEncoder);
        };

        // Not implemented for Metal
        //callbacks.Impl_ScreenshotRgb = []() { ...;};

        callbacks.Impl_Frame_3D_ClearColor = [](ImVec4 clear_color)
        {
            gSdlMetalGlobals.clear_color = clear_color;
        };

        callbacks.Impl_Shutdown_3D = []
        {
            ImGui_ImplMetal_Shutdown();
        };
        return callbacks;
    }

#endif // #ifdef HELLOIMGUI_USE_SDL2


} // namespace HelloImGui

#endif // HELLOIMGUI_HAS_METAL