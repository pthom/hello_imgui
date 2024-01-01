#ifdef HELLOIMGUI_USE_GLFW

#ifdef HELLOIMGUI_HAS_OPENGL
//#include "hello_imgui/hello_imgui_include_opengl.h"
#include <backends/imgui_impl_opengl3.h>
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
#include "hello_imgui/internal/stb_image.h"

#include "backend_window_helper/glfw_window_helper.h"
#include "runner_glfw3.h"
#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <imgui.h>


namespace HelloImGui
{
#ifdef HELLOIMGUI_HAS_OPENGL
    BackendApi::OpenGlSetupGlfw gOpenGlHelper;
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

    void RunnerGlfw3::Impl_CreateWindow()
    {
        BackendApi::BackendOptions backendOptions;
#ifdef HELLOIMGUI_HAS_METAL
        backendOptions.backend3DMode = BackendApi::Backend3dMode::Metal;
#endif
        mWindow = mBackendWindowHelper->CreateWindow(params.appWindowParams, backendOptions);
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
        glfwSwapBuffers((GLFWwindow *)mWindow);
#endif
#ifdef HELLOIMGUI_HAS_METAL
        SwapMetalBuffers();
#endif
#ifdef HELLOIMGUI_HAS_VULKAN
        SwapVulkanBuffers();
#endif
#ifdef HELLOIMGUI_HAS_DIRECTX11
        SwapDx11Buffers();
#endif
//#ifdef HELLOIMGUI_HAS_DIRECTX12
//        SwapDx12Buffers();
//#endif
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
    void RunnerGlfw3::Impl_LinkPlatformAndRenderBackends()
    {
        ImGui_ImplGlfw_InitForOpenGL((GLFWwindow *)mWindow, true);
        ImGui_ImplOpenGL3_Init(Impl_GlslVersion().c_str());
    }

    void RunnerGlfw3::Impl_InitRenderBackendCallbacks()
    {
        mRenderingBackendCallbacks = CreateBackendCallbacks_OpenGl3();
    }

    void RunnerGlfw3::Impl_CreateGlContext()
    {
        glfwMakeContextCurrent((GLFWwindow *) mWindow); // OpenGl!
        glfwSwapInterval(1);  // Enable vsync (openGL only, not vulkan)
    }

    void RunnerGlfw3::Impl_Select_Gl_Version() { gOpenGlHelper.SelectOpenGlVersion(); }

    std::string RunnerGlfw3::Impl_GlslVersion() { return gOpenGlHelper.GlslVersion(); }

    void RunnerGlfw3::Impl_InitGlLoader() { gOpenGlHelper.InitGlLoader(); }

#elif defined(HELLOIMGUI_HAS_METAL)

    void RunnerGlfw3::Impl_InitRenderBackendCallbacks()
    {
        mRenderingBackendCallbacks = CreateBackendCallbacks_GlfwMetal();
    }
    void RunnerGlfw3::Impl_LinkPlatformAndRenderBackends()
    {
        PrepareGlfwForMetal((GLFWwindow *) mWindow);
    }

#elif defined(HELLOIMGUI_HAS_VULKAN)
    void RunnerGlfw3::Impl_InitRenderBackendCallbacks()
    {
        // Below, call of RenderingCallbacks_LinkWindowingToRenderingBackend
        mRenderingBackendCallbacks = CreateBackendCallbacks_GlfwVulkan();
    }
    void RunnerGlfw3::Impl_LinkPlatformAndRenderBackends()
    {
        // Below, call of RenderingCallbacks_LinkWindowingToRenderingBackend
        PrepareGlfwForVulkan((GLFWwindow *) mWindow);
    }

#elif defined(HELLOIMGUI_HAS_DIRECTX11)
    void RunnerGlfw3::Impl_InitRenderBackendCallbacks()
    {
        // Below, call of RenderingCallbacks_LinkWindowingToRenderingBackend
        mRenderingBackendCallbacks = CreateBackendCallbacks_GlfwDx11();
    }
    void RunnerGlfw3::Impl_LinkPlatformAndRenderBackends()
    {
        // Below, call of RenderingCallbacks_LinkWindowingToRenderingBackend
        PrepareGlfwForDx11((GLFWwindow *) mWindow);
    }

//#elif defined(HELLOIMGUI_HAS_DIRECTX12)
//    void RunnerGlfw3::Impl_InitRenderBackendCallbacks()
//    {
//        // Below, call of RenderingCallbacks_LinkWindowingToRenderingBackend
//        mRenderingBackendCallbacks = CreateBackendCallbacks_GlfwDx12();
//    }
//    void RunnerGlfw3::Impl_LinkPlatformAndRenderBackends()
//    {
//        // Below, call of RenderingCallbacks_LinkWindowingToRenderingBackend
//        PrepareGlfwForDx12((GLFWwindow *) mWindow);
//    }

#endif


}  // namespace HelloImGui
#endif  // #ifdef HELLOIMGUI_USE_GLFW
