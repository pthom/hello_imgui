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
    GlfwMetalGlobals gGlfwMetalGlobals;

    void PrepareGlfwForMetal_WithWindow_PreImGuiInit(GLFWwindow* glfwWindow)
    {
        gGlfwMetalGlobals.glfwWindow = glfwWindow;
//        gGlfwMetalGlobals.sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
//        if (gGlfwMetalGlobals.sdlRenderer == nullptr)
//        {
//            bool Error_SdlCreateRenderer_For_Metal = false;
//            IM_ASSERT(Error_SdlCreateRenderer_For_Metal);
//            exit(-3);
//        }

        // Setup Platform/Renderer backends
//        gGlfwMetalGlobals.caMetalLayer = (__bridge CAMetalLayer*)SDL_RenderGetMetalLayer(gGlfwMetalGlobals.sdlRenderer);
//        gGlfwMetalGlobals.caMetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;

        gGlfwMetalGlobals.mtlDevice = MTLCreateSystemDefaultDevice();
        gGlfwMetalGlobals.mtlCommandQueue = [gGlfwMetalGlobals.mtlDevice newCommandQueue];
    }

    void PrepareGlfwForMetal_PosImGuiInit()
    {
        ImGui_ImplGlfw_InitForOther(gGlfwMetalGlobals.glfwWindow, true);
        ImGui_ImplMetal_Init(gGlfwMetalGlobals.mtlDevice);

        NSWindow *nswin = glfwGetCocoaWindow(gGlfwMetalGlobals.glfwWindow);
        gGlfwMetalGlobals.caMetalLayer = [CAMetalLayer layer];
        gGlfwMetalGlobals.caMetalLayer.device = gGlfwMetalGlobals.mtlDevice;
        gGlfwMetalGlobals.caMetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        nswin.contentView.layer = gGlfwMetalGlobals.caMetalLayer;
        nswin.contentView.wantsLayer = YES;

        gGlfwMetalGlobals.mtlRenderPassDescriptor = [MTLRenderPassDescriptor new];
    }

    void SwapGlfwMetalBuffers()
    {
        [gGlfwMetalGlobals.mtlRenderCommandEncoder popDebugGroup];
        [gGlfwMetalGlobals.mtlRenderCommandEncoder endEncoding];

        [gGlfwMetalGlobals.mtlCommandBuffer presentDrawable:gGlfwMetalGlobals.caMetalDrawable];
        [gGlfwMetalGlobals.mtlCommandBuffer commit];
    }

    RenderingCallbacks CreateBackendCallbacks_GlfwMetal()
    {
        RenderingCallbacks callbacks;

        callbacks.Impl_NewFrame_3D = []
        {
            auto Vec4_To_Array = [](ImVec4 v) { return std::array<float, 4>{ v.x, v.y, v.z, v.w }; };

            int width, height;
            glfwGetFramebufferSize(gGlfwMetalGlobals.glfwWindow, &width, &height);
            gGlfwMetalGlobals.caMetalLayer.drawableSize = CGSizeMake(width, height);
            gGlfwMetalGlobals.caMetalDrawable = [gGlfwMetalGlobals.caMetalLayer nextDrawable];

            gGlfwMetalGlobals.mtlCommandBuffer = [gGlfwMetalGlobals.mtlCommandQueue commandBuffer];
            auto clearColor = Vec4_To_Array(HelloImGui::GetRunnerParams()->imGuiWindowParams.backgroundColor);
            gGlfwMetalGlobals.mtlRenderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clearColor[0] * clearColor[3], clearColor[1] * clearColor[3], clearColor[2] * clearColor[3], clearColor[3]);
            gGlfwMetalGlobals.mtlRenderPassDescriptor.colorAttachments[0].texture = gGlfwMetalGlobals.caMetalDrawable.texture;
            gGlfwMetalGlobals.mtlRenderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
            gGlfwMetalGlobals.mtlRenderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
            gGlfwMetalGlobals.mtlRenderCommandEncoder = [
                gGlfwMetalGlobals.mtlCommandBuffer renderCommandEncoderWithDescriptor:gGlfwMetalGlobals.mtlRenderPassDescriptor];
            [gGlfwMetalGlobals.mtlRenderCommandEncoder pushDebugGroup:@"ImGui demo"];

            // Start the Dear ImGui frame
            ImGui_ImplMetal_NewFrame(gGlfwMetalGlobals.mtlRenderPassDescriptor);
            ImGui_ImplGlfw_NewFrame();
        };

        callbacks.Impl_RenderDrawData_To_3D = []
        {
            ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), gGlfwMetalGlobals.mtlCommandBuffer, gGlfwMetalGlobals.mtlRenderCommandEncoder);
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

    GlfwMetalGlobals& GetGlfwMetalGlobals()
    {
        return gGlfwMetalGlobals;
    }


} // namespace HelloImGui

#endif // HELLOIMGUI_USE_GLFW3
#endif // HELLOIMGUI_HAS_METAL