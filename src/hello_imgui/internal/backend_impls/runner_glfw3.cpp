#ifdef HELLOIMGUI_USE_GLFW3

#ifdef HELLOIMGUI_HAS_OPENGL
//#include "hello_imgui/hello_imgui_include_opengl.h"
#include "imgui_impl_opengl3.h"
#include "opengl_setup_helper/opengl_setup_glfw.h"
#include "opengl_setup_helper/opengl_screenshot.h"
#include "rendering_opengl3.h"
#endif
#ifdef HELLOIMGUI_HAS_METAL
#include "rendering_metal.h"
#endif
#ifdef HELLOIMGUI_HAS_VULKAN
#include "rendering_vulkan.h"
#endif

// DirectX unsupported with Glfw
#ifdef HELLOIMGUI_HAS_DIRECTX11
#include "rendering_dx11.h"
#endif
//#ifdef HELLOIMGUI_HAS_DIRECTX12
//#include "rendering_dx12.h"
//#endif

#include "hello_imgui/hello_imgui.h"
#include "stb_image.h"

#include "backend_window_helper/glfw_window_helper.h"
#include "runner_glfw3.h"
#include <GLFW/glfw3.h>
#include "imgui_impl_glfw.h"
#include <imgui.h>


namespace HelloImGui
{
#ifdef HELLOIMGUI_HAS_OPENGL
    BackendApi::OpenGlSetupGlfw gOpenGlSetupGlfw;
#endif

    static void glfw_error_callback(int error, const char* description)
    {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    }

    RunnerGlfw3::RunnerGlfw3(RunnerParams & runnerParams)
        : AbstractRunner(runnerParams)
    {
        mBackendWindowHelper = std::make_unique<BackendApi::GlfwWindowHelper>();
    }

    void RunnerGlfw3::Impl_InitPlatformBackend()
    {
        glfwSetErrorCallback(glfw_error_callback);
        #ifdef __APPLE__
            // prevent glfw from changing the current dir on macOS.
            // This glfw behaviour is for Mac only, and interferes with our multiplatform assets handling
            glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
        #endif
        bool glfwInitSuccess = glfwInit();
        (void) glfwInitSuccess;
        IM_ASSERT(glfwInitSuccess);
    }

    void RunnerGlfw3::Impl_CreateWindow(std::function<void()> renderCallbackDuringResize)
    {
        BackendApi::BackendOptions backendOptions;
        backendOptions.rendererBackendType = params.rendererBackendType;
        mWindow = mBackendWindowHelper->CreateWindow(params.appWindowParams, backendOptions, renderCallbackDuringResize);
        params.backendPointers.glfwWindow = mWindow;
    }

    void RunnerGlfw3::Impl_PollEvents()
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants
        // to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main
        // application. Generally you may always pass all inputs to dear imgui, and hide them from your
        // application based on those two flags.
        glfwPollEvents();
        bool exitRequired = (glfwWindowShouldClose((GLFWwindow *)mWindow) != 0);
        if (exitRequired)
            params.appShallExit = true;
    }

    void RunnerGlfw3::Impl_NewFrame_PlatformBackend() { ImGui_ImplGlfw_NewFrame(); }

    void RunnerGlfw3::Impl_UpdateAndRenderAdditionalPlatformWindows()
    {
        #ifdef HELLOIMGUI_HAS_OPENGL
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
        #endif
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        #ifdef HELLOIMGUI_HAS_OPENGL
            glfwMakeContextCurrent(backup_current_context);
        #endif
    }

    void RunnerGlfw3::Impl_Cleanup()
    {
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow((GLFWwindow *)mWindow);
        glfwTerminate();
    }

    void RunnerGlfw3::Impl_SwapBuffers()
    {
        // Note: call of RenderingCallbacks_Impl_SwapBuffers
        #ifdef HELLOIMGUI_HAS_OPENGL
            if (params.rendererBackendType == RendererBackendType::OpenGL3)
                glfwSwapBuffers((GLFWwindow *)mWindow);
        #endif
        #ifdef HELLOIMGUI_HAS_METAL
            if (params.rendererBackendType == RendererBackendType::Metal)
                SwapMetalBuffers();
        #endif
        #ifdef HELLOIMGUI_HAS_VULKAN
            if (params.rendererBackendType == RendererBackendType::Vulkan)
                SwapVulkanBuffers();
        #endif
        #ifdef HELLOIMGUI_HAS_DIRECTX11
            if (params.rendererBackendType == RendererBackendType::DirectX11)
                SwapDx11Buffers();
        #endif
        #ifdef HELLOIMGUI_HAS_DIRECTX12
            // if (params.rendererBackendType == RendererBackendType::DirectX12)
            //     SwapDx12Buffers();
        #endif
    }

    void RunnerGlfw3::Impl_SetWindowIcon()
    {
        std::string iconFile = "app_settings/icon.png";
        if (!HelloImGui::AssetExists(iconFile))
            return;

        auto imageAsset = HelloImGui::LoadAssetFileData(iconFile.c_str());
        int width, height, channels;
        unsigned char* image = stbi_load_from_memory(
            (stbi_uc *)imageAsset.data, (int)imageAsset.dataSize , &width, &height, &channels, 4); // force RGBA channels

        if (image)
        {
            GLFWimage icons[1];
            icons[0].width = width;
            icons[0].height = height;
            icons[0].pixels = image; // GLFWImage expects an array of pixels (unsigned char *)

            glfwSetWindowIcon((GLFWwindow*)mWindow, 1, icons);

            stbi_image_free(image);
        }
        else
            HIMG_LOG("RunnerGlfwOpenGl3::Impl_SetWindowIcon: Failed to load window icon: " + iconFile);
        HelloImGui::FreeAssetFileData(&imageAsset);
    }


    ///////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Link with Rendering Backends (OpenGL, Vulkan, ...)
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////
#if defined(HELLOIMGUI_HAS_OPENGL)
    void Impl_LinkPlatformAndRenderBackends_GlfwOpenGl(const RunnerGlfw3& runner)
    {
        ImGui_ImplGlfw_InitForOpenGL((GLFWwindow *)runner.mWindow, true);
        ImGui_ImplOpenGL3_Init(runner.Impl_GlslVersion().c_str());
    }

    void RunnerGlfw3::Impl_CreateGlContext()
    {
        glfwMakeContextCurrent((GLFWwindow *) mWindow); // OpenGl!
        glfwSwapInterval(1);  // Enable vsync (openGL only, not vulkan)
    }

    void RunnerGlfw3::Impl_Select_Gl_Version()
    {
        auto openGlOptions = gOpenGlSetupGlfw.OpenGlOptionsWithUserSettings();
        gOpenGlSetupGlfw.SelectOpenGlVersion(openGlOptions);
    }

    std::string RunnerGlfw3::Impl_GlslVersion() const
    {
        return std::string("#version ") + gOpenGlSetupGlfw.OpenGlOptionsWithUserSettings().GlslVersion;
    }

    void RunnerGlfw3::Impl_InitGlLoader() { gOpenGlSetupGlfw.InitGlLoader(); }
#endif // HELLOIMGUI_HAS_OPENGL

#if defined(HELLOIMGUI_HAS_METAL)
    void Impl_LinkPlatformAndRenderBackends_GlfwMetal(const RunnerGlfw3& runner)
    {
        PrepareGlfwForMetal((GLFWwindow *) runner.mWindow, runner.params.rendererBackendOptions);
    }
#endif

#if defined(HELLOIMGUI_HAS_VULKAN)
    void Impl_LinkPlatformAndRenderBackends_GlfwVulkan(const RunnerGlfw3& runner)
    {
        // Below, call of RenderingCallbacks_LinkWindowingToRenderingBackend
        PrepareGlfwForVulkan((GLFWwindow *) runner.mWindow);
    }
#endif // HELLOIMGUI_HAS_VULKAN

#if defined(HELLOIMGUI_HAS_DIRECTX11)
    void Impl_LinkPlatformAndRenderBackends_GlfwDirectX11(const RunnerGlfw3& runner)
    {
        // Below, call of RenderingCallbacks_LinkWindowingToRenderingBackend
        PrepareGlfwForDx11((GLFWwindow *) runner.mWindow);
    }
#endif

#if defined(HELLOIMGUI_HAS_DIRECTX12)
//    void Impl_LinkPlatformAndRenderBackends_GlfwDirectX12(const RunnerGlfw3& runner)
//    {
//        // Below, call of RenderingCallbacks_LinkWindowingToRenderingBackend
//        PrepareGlfwForDx12((GLFWwindow *) runner.mWindow);
//    }
#endif

    void RunnerGlfw3::Impl_LinkPlatformAndRenderBackends()
    {
        auto& self = *this;
        if (params.rendererBackendType == RendererBackendType::OpenGL3)
        {
            #ifdef HELLOIMGUI_HAS_OPENGL
                Impl_LinkPlatformAndRenderBackends_GlfwOpenGl(self);
            #else
                IM_ASSERT(false && "OpenGL3 not supported");
            #endif
        }
        else if(params.rendererBackendType == RendererBackendType::Metal)
        {
            #ifdef HELLOIMGUI_HAS_METAL
                Impl_LinkPlatformAndRenderBackends_GlfwMetal(self);
            #else
                IM_ASSERT(false && "Metal not supported");
            #endif
        }
        else if(params.rendererBackendType == RendererBackendType::Vulkan)
        {
            #ifdef HELLOIMGUI_HAS_VULKAN
                Impl_LinkPlatformAndRenderBackends_GlfwVulkan(self);
            #else
                IM_ASSERT(false && "Vulkan not supported");
            #endif
        }
        else if(params.rendererBackendType == RendererBackendType::DirectX11)
        {
            #ifdef HELLOIMGUI_HAS_DIRECTX11
                Impl_LinkPlatformAndRenderBackends_GlfwDirectX11(self);
            #else
                IM_ASSERT(false && "DirectX11 not supported");
            #endif
        }
        else if(params.rendererBackendType == RendererBackendType::DirectX12)
        {
            #ifdef HELLOIMGUI_HAS_DIRECTX12
                // Impl_LinkPlatformAndRenderBackends_GlfwDirectX12(self);
                IM_ASSERT(false && "The combination Glfw + DirectX12 is not supported");
            #else
                IM_ASSERT(false && "DirectX12 not supported");
            #endif
        }
    }


}  // namespace HelloImGui
#endif  // #ifdef HELLOIMGUI_USE_GLFW3
