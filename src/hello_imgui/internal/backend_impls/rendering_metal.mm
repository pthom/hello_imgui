#ifdef HELLOIMGUI_HAS_METAL
#include "rendering_metal.h"

#include <backends/imgui_impl_metal.h>
#include "hello_imgui/hello_imgui.h"


namespace HelloImGui
{
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
    }

    RenderingCallbacksPtr PrepareBackendCallbacksCommon()
    {
        auto callbacks = std::make_shared<RenderingCallbacks>();

        callbacks->Impl_NewFrame_3D = [callbacks] // Capture by value required, since we are using the callbacks shared_ptr
        {
            auto& gMetalGlobals = GetMetalGlobals();

            auto Vec4_To_Array = [](ImVec4 v) { return std::array<float, 4>{ v.x, v.y, v.z, v.w }; };

            //
            // New Frame + Clear color
            //
            ScreenSize frameBufferSize = callbacks->Impl_GetFrameBufferSize();
            gMetalGlobals.caMetalLayer.drawableSize = CGSizeMake(frameBufferSize[0], frameBufferSize[1]);
            gMetalGlobals.caMetalDrawable = [gMetalGlobals.caMetalLayer nextDrawable];

            gMetalGlobals.mtlCommandBuffer = [gMetalGlobals.mtlCommandQueue commandBuffer];
            auto clearColor = Vec4_To_Array(HelloImGui::GetRunnerParams()->imGuiWindowParams.backgroundColor);
            gMetalGlobals.mtlRenderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clearColor[0] * clearColor[3], clearColor[1] * clearColor[3], clearColor[2] * clearColor[3], clearColor[3]);
            gMetalGlobals.mtlRenderPassDescriptor.colorAttachments[0].texture = gMetalGlobals.caMetalDrawable.texture;
            gMetalGlobals.mtlRenderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
            gMetalGlobals.mtlRenderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
            gMetalGlobals.mtlRenderCommandEncoder = [
                gMetalGlobals.mtlCommandBuffer renderCommandEncoderWithDescriptor:gMetalGlobals.mtlRenderPassDescriptor];
            [gMetalGlobals.mtlRenderCommandEncoder pushDebugGroup:@"ImGui demo"];

            // Start the Dear ImGui frame
            ImGui_ImplMetal_NewFrame(gMetalGlobals.mtlRenderPassDescriptor);
        };

        callbacks->Impl_RenderDrawData_To_3D = []
        {
            auto& gMetalGlobals = GetMetalGlobals();
            ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), gMetalGlobals.mtlCommandBuffer, gMetalGlobals.mtlRenderCommandEncoder);
        };

        // Not implemented for Metal
        //callbacks.Impl_ScreenshotRgb = []() { ...;};

        // This is done at Impl_NewFrame_3D
        callbacks->Impl_Frame_3D_ClearColor = [](ImVec4 clearColor) { };

        callbacks->Impl_Shutdown_3D = []
        {
            ImGui_ImplMetal_Shutdown();
        };

        return callbacks;
    }

}

#endif // HELLOIMGUI_HAS_METAL