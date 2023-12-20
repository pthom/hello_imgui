#ifdef HELLOIMGUI_HAS_METAL
#ifdef HELLOIMGUI_USE_GLFW3

#include "rendering_metal.h"

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#include <backends/imgui_impl_metal.h>
#include <array>

#include "hello_imgui/hello_imgui.h"

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <backends/imgui_impl_glfw.h>


namespace HelloImGui
{

    GlfwMetalGlobals& GetGlfwMetalGlobals()
    {
            static GlfwMetalGlobals sGlfwMetalGlobals;
            return sGlfwMetalGlobals;
    }

    void PrepareGlfwForMetal_WithWindow_PreImGuiInit(GLFWwindow* glfwWindow)
    {
        auto& gMetalGlobals = GetMetalGlobals();
        auto& gGlfwMetalGlobals = GetGlfwMetalGlobals();

        gGlfwMetalGlobals.glfwWindow = glfwWindow;
        gGlfwMetalGlobals.mtlDevice = MTLCreateSystemDefaultDevice();
        gMetalGlobals.mtlCommandQueue = [gGlfwMetalGlobals.mtlDevice newCommandQueue];
    }

    void PrepareGlfwForMetal_PosImGuiInit()
    {
        auto& gMetalGlobals = GetMetalGlobals();

        auto& gGlfwMetalGlobals = GetGlfwMetalGlobals();
        ImGui_ImplGlfw_InitForOther(gGlfwMetalGlobals.glfwWindow, true);
        ImGui_ImplMetal_Init(gGlfwMetalGlobals.mtlDevice);

        NSWindow *nswin = glfwGetCocoaWindow(gGlfwMetalGlobals.glfwWindow);
        gMetalGlobals.caMetalLayer = [CAMetalLayer layer];
        gMetalGlobals.caMetalLayer.device = gGlfwMetalGlobals.mtlDevice;
        gMetalGlobals.caMetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        nswin.contentView.layer = gMetalGlobals.caMetalLayer;
        nswin.contentView.wantsLayer = YES;

        gMetalGlobals.mtlRenderPassDescriptor = [MTLRenderPassDescriptor new];
    }

    void SwapGlfwMetalBuffers()
    {
        auto& gMetalGlobals = GetMetalGlobals();

        [gMetalGlobals.mtlRenderCommandEncoder popDebugGroup];
        [gMetalGlobals.mtlRenderCommandEncoder endEncoding];

        [gMetalGlobals.mtlCommandBuffer presentDrawable:gMetalGlobals.caMetalDrawable];
        [gMetalGlobals.mtlCommandBuffer commit];
    }

    RenderingCallbacks CreateBackendCallbacks_GlfwMetal()
    {
        RenderingCallbacks callbacks;

        callbacks.Impl_NewFrame_3D = []
        {
            auto& gMetalGlobals = GetMetalGlobals();
            auto& gGlfwMetalGlobals = GetGlfwMetalGlobals();

            auto Vec4_To_Array = [](ImVec4 v) { return std::array<float, 4>{ v.x, v.y, v.z, v.w }; };

            int width, height;
            glfwGetFramebufferSize(gGlfwMetalGlobals.glfwWindow, &width, &height);
            gMetalGlobals.caMetalLayer.drawableSize = CGSizeMake(width, height);
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
            ImGui_ImplGlfw_NewFrame();
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

#endif // HELLOIMGUI_USE_GLFW3
#endif // HELLOIMGUI_HAS_METAL