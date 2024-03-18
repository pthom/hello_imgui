#ifdef HELLOIMGUI_HAS_METAL
#include "rendering_metal.h"

#include "imgui_impl_metal.h"
#include "hello_imgui/hello_imgui.h"

#import <Cocoa/Cocoa.h>

namespace HelloImGui
{
    bool hasEdrSupport()
    {
        NSArray<NSScreen *> * screens = [NSScreen screens];
        bool buffer_edr = false;

        for (NSScreen * screen in screens) {
            if ([screen respondsToSelector:@selector
                        (maximumPotentialExtendedDynamicRangeColorComponentValue)]) {
                if ([screen maximumPotentialExtendedDynamicRangeColorComponentValue] >= 2.f)
                    buffer_edr = true;
            }
        }

        return buffer_edr;
    }

    MetalGlobals& GetMetalGlobals()
    {
        static MetalGlobals sMetalGlobals;
        return sMetalGlobals;
    }

    //  Impl of RenderingCallbacks_Impl_SwapBuffers
    void SwapMetalBuffers()
    {
        auto& gMetalGlobals = GetMetalGlobals();

        [gMetalGlobals.mtlRenderCommandEncoder popDebugGroup];
        [gMetalGlobals.mtlRenderCommandEncoder endEncoding];

        [gMetalGlobals.mtlCommandBuffer presentDrawable:gMetalGlobals.caMetalDrawable];
        [gMetalGlobals.mtlCommandBuffer commit];

        // Release may be needed: these were created by Metal during this frame
        // but not inside an autoreleasepool block
        [gMetalGlobals.caMetalDrawable release];
        [gMetalGlobals.mtlCommandBuffer release];
        [gMetalGlobals.mtlRenderCommandEncoder release];

        gMetalGlobals.caMetalDrawable = nullptr;
        gMetalGlobals.mtlCommandBuffer = nullptr;
        gMetalGlobals.mtlRenderCommandEncoder = nullptr;
    }

    RenderingCallbacksPtr PrepareBackendCallbacksCommon()
    {
        auto callbacks = std::make_shared<RenderingCallbacks>();

        callbacks->Impl_NewFrame_3D = [callbacks] // Capture by value required, since we are using the callbacks shared_ptr
        {
            auto& gMetalGlobals = GetMetalGlobals();

            //
            // New Frame + Clear color
            //
            ScreenSize frameBufferSize = callbacks->Impl_GetFrameBufferSize();
            gMetalGlobals.caMetalLayer.drawableSize = CGSizeMake(frameBufferSize[0], frameBufferSize[1]);
            gMetalGlobals.caMetalDrawable = [gMetalGlobals.caMetalLayer nextDrawable];

            auto& clearColor = HelloImGui::GetRunnerParams()->imGuiWindowParams.backgroundColor;
            gMetalGlobals.mtlRenderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);
            gMetalGlobals.mtlRenderPassDescriptor.colorAttachments[0].texture = gMetalGlobals.caMetalDrawable.texture;
            gMetalGlobals.mtlRenderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
            gMetalGlobals.mtlRenderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

            // Start the Dear ImGui frame
            ImGui_ImplMetal_NewFrame(gMetalGlobals.mtlRenderPassDescriptor);
        };

        callbacks->Impl_RenderDrawData_To_3D = []
        {
            auto& gMetalGlobals = GetMetalGlobals();

            gMetalGlobals.mtlCommandBuffer = [gMetalGlobals.mtlCommandQueue commandBuffer];
            gMetalGlobals.mtlRenderCommandEncoder = [
                gMetalGlobals.mtlCommandBuffer renderCommandEncoderWithDescriptor:gMetalGlobals.mtlRenderPassDescriptor];
            [gMetalGlobals.mtlRenderCommandEncoder pushDebugGroup:@"ImGui demo"];

            ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), gMetalGlobals.mtlCommandBuffer, gMetalGlobals.mtlRenderCommandEncoder);
        };

        // Not implemented for Metal
        //callbacks.Impl_ScreenshotRgb = []() { ...;};

        callbacks->Impl_Frame_3D_ClearColor = [](ImVec4 clearColor) {
            auto& gMetalGlobals = GetMetalGlobals();
            gMetalGlobals.mtlRenderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
        };

        callbacks->Impl_Shutdown_3D = []
        {
            ImGui_ImplMetal_Shutdown();
        };

        callbacks->Impl_CreateFontTexture = []
        {
            auto& gMetalGlobals = GetMetalGlobals();
            ImGui_ImplMetal_CreateFontsTexture(gMetalGlobals.mtlDevice);
        };
        callbacks->Impl_DestroyFontTexture = ImGui_ImplMetal_DestroyFontsTexture;

        return callbacks;
    }

}

#endif // HELLOIMGUI_HAS_METAL