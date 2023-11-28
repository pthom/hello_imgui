#ifdef HELLOIMGUI_USE_GLFW_OPENGL3

#include "hello_imgui/hello_imgui_include_opengl.h"
#include "hello_imgui/hello_imgui.h"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>
#include <stdexcept>
#include "hello_imgui/hello_imgui_error.h"
#include "runner_glfw_opengl3.h"
#include "backend_window_helper/glfw_window_helper.h"
#include "opengl_setup_helper/opengl_setup_glfw.h"
#include "opengl_setup_helper/opengl_screenshot.h"


namespace HelloImGui
{
    BackendApi::OpenGlSetupGlfw gOpenGlHelper;


    static void glfw_error_callback(int error, const char* description)
    {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    }

    RunnerGlfwOpenGl3::RunnerGlfwOpenGl3(RunnerParams & runnerParams)
        : AbstractRunner(runnerParams)
    {
        mBackendWindowHelper = std::make_unique<BackendApi::GlfwWindowHelper>();
    }

    void RunnerGlfwOpenGl3::Impl_InitBackend()
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

    void RunnerGlfwOpenGl3::Impl_Select_Gl_Version()
    {
        gOpenGlHelper.SelectOpenGlVersion();
    }

    std::string RunnerGlfwOpenGl3::Impl_GlslVersion()
    {
        return gOpenGlHelper.GlslVersion();
    }

    void RunnerGlfwOpenGl3::Impl_CreateWindow()
    {
        BackendApi::BackendOptions backendOptions;

        mWindow = mBackendWindowHelper->CreateWindow(params.appWindowParams, backendOptions);
        params.backendPointers.glfwWindow = mWindow;
    }

    void RunnerGlfwOpenGl3::Impl_CreateGlContext()
    {
        glfwMakeContextCurrent((GLFWwindow *) mWindow); // OpenGl!
        glfwSwapInterval(1);  // Enable vsync (openGL only, not vulkan)
    }


    void RunnerGlfwOpenGl3::Impl_InitGlLoader()
    {
        gOpenGlHelper.InitGlLoader();
    }

    void RunnerGlfwOpenGl3::Impl_SetupPlatformRendererBindings()
    {
        ImGui_ImplGlfw_InitForOpenGL((GLFWwindow *)mWindow, true);
        ImGui_ImplOpenGL3_Init(Impl_GlslVersion().c_str());
    }

    void RunnerGlfwOpenGl3::Impl_PollEvents()
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

    void RunnerGlfwOpenGl3::Impl_NewFrame_3D() { ImGui_ImplOpenGL3_NewFrame(); }

    void RunnerGlfwOpenGl3::Impl_NewFrame_Backend() { ImGui_ImplGlfw_NewFrame(); }

    void RunnerGlfwOpenGl3::Impl_Frame_3D_ClearColor()
    {
        auto& io = ImGui::GetIO();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        ImVec4 clear_color = params.imGuiWindowParams.backgroundColor;
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void RunnerGlfwOpenGl3::Impl_RenderDrawData_To_3D() { ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); }

    void RunnerGlfwOpenGl3::Impl_UpdateAndRenderAdditionalPlatformWindows()
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }

    void RunnerGlfwOpenGl3::Impl_Cleanup()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow((GLFWwindow *)mWindow);
        glfwTerminate();
    }

    void RunnerGlfwOpenGl3::Impl_SwapBuffers() { glfwSwapBuffers((GLFWwindow *)mWindow); }

    ImageBuffer RunnerGlfwOpenGl3::Impl_ScreenshotRgb()
    {
        return OpenglScreenshotRgb();
    }

}  // namespace HelloImGui
#endif  // #ifdef HELLOIMGUI_USE_GLFW_OPENGL3
