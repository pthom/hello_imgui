#ifdef HELLOIMGUI_HAS_METAL
#include "rendering_metal.h"

//#import <Metal/Metal.h>
//#import <QuartzCore/QuartzCore.h>
#include <backends/imgui_impl_metal.h>


namespace HelloImGui
{
/*

//
// Init SDL
//

// Inform SDL that we will be using metal for rendering. Without this hint initialization of metal renderer may fail.
SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");

SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
if (renderer == nullptr)
{
    printf("Error creating renderer: %s\n", SDL_GetError());
    return -3;
}

//
// Link SDL / Metal
//

// Setup Platform/Renderer backends
CAMetalLayer* layer = (__bridge CAMetalLayer*)SDL_RenderGetMetalLayer(renderer);
layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
ImGui_ImplMetal_Init(layer.device);
ImGui_ImplSDL2_InitForMetal(window);

id<MTLCommandQueue> commandQueue = [layer.device newCommandQueue];
MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor new];

//
// New Frame + Clear color
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
ImGui_ImplMetal_Shutdown();

*/

    RenderingCallbacks CreateBackendCallbacks_Metal()
{
    RenderingCallbacks callbacks;

    callbacks.Impl_NewFrame_3D = [] {
    };

    callbacks.Impl_RenderDrawData_To_3D = [] {
    };

    // Not implemented for Metal
    //callbacks.Impl_ScreenshotRgb = []() { ...;};

    callbacks.Impl_Frame_3D_ClearColor = [](ImVec4 clear_color) {
    };

    callbacks.Impl_Shutdown = [] {
        //            ImGui_ImplMetal_Shutdown();
    };
    return callbacks;
}
}

#endif // HELLOIMGUI_HAS_METAL