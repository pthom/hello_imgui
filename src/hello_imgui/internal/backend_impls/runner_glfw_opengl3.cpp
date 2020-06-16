#ifdef HELLOIMGUI_USE_GLFW_OPENGL3

#if defined(HELLOIMGUI_USE_GLES3)
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#elif defined(HELLOIMGUI_USE_GLAD)
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
#include <imgui.h>
#include <stdexcept>

#include "runner_glfw_opengl3.h"

namespace HelloImGui
{
    static void glfw_error_callback(int error, const char* description)
    {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    }

    void RunnerGlfwOpenGl3::Impl_InitBackend()
    {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
            throw std::runtime_error("RunnerGlfwOpenGl3::Impl_InitBackend failed");
    }

    void RunnerGlfwOpenGl3::Impl_Select_Gl_Version()
    {
#if defined(HELLOIMGUI_USE_GLES3)
        {
            //test
            throw std::logic_error("RunnerGlfwOpenGl3 needs implementation for GLES !");
//            SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 3);
//            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
//                                SDL_GL_CONTEXT_PROFILE_ES);
//            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        }
#elif defined(__APPLE__)
        {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
        }
#else
        {
            // GL 3.2+
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
        }
#endif
    }

    std::string RunnerGlfwOpenGl3::Impl_GlslVersion()
    {
#if defined(HELLOIMGUI_USE_GLES3)
        const char* glsl_version = "#version 300es";
#elif defined(__APPLE__)
        const char* glsl_version = "#version 150";
#else
        // GLSL 130
        const char* glsl_version = "#version 130";
#endif
        return glsl_version;
    }

    void RunnerGlfwOpenGl3::Impl_CreateWindowAndContext()
    {
        const AppWindowParams& backendWindowParams = params.appWindowParams;
        ImVec2 windowSize = backendWindowParams.windowSize;
        ImVec2 windowPosition = backendWindowParams.windowPosition;

        // Check if full screen mode is requested
        if (backendWindowParams.fullScreen)
        {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            glfwWindowHint(GLFW_RED_BITS, mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
            mWindow = glfwCreateWindow(
                mode->width, mode->height, backendWindowParams.windowTitle.c_str(),
                monitor, nullptr);
        }
        else
        {
            mWindow = glfwCreateWindow(
                (int)windowSize.x, (int)windowSize.y, backendWindowParams.windowTitle.c_str(), NULL, NULL);
        }
        if (windowPosition.x >= -10000.f)
            glfwSetWindowPos(mWindow, (int)windowPosition.x, (int)windowPosition.y);

        if (mWindow == NULL)
        {
            glfwTerminate();
            throw std::runtime_error("RunnerGlfwOpenGl3::Impl_CreateWindowAndContext failed");
        }
        glfwMakeContextCurrent(mWindow);
        glfwSwapInterval(1);  // Enable vsync
    }

    void RunnerGlfwOpenGl3::Impl_InitGlLoader()
    {
#ifndef __EMSCRIPTEN__
        //  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        //    throw std::runtime_error("gladLoadGLLoader: Failed");
        //  if (!GLAD_GL_VERSION_3_3)
        //    throw(std::runtime_error("GLAD could not initialize OpenGl 3.3"));

        // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
        bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
        bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
        bool err = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0;
#else
        bool err = false;  // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires
                           // some form of initialization.
#endif
        if (err)
        {
            throw std::runtime_error("RunnerGlfwOpenGl3::Impl_InitGlLoader(): Failed to initialize OpenGL loader!");
        }
#endif  // #ifndef __EMSCRIPTEN__

#ifdef GLAD_DEBUG
        glad_set_pre_callback(glad_pre_call_callback);
        glad_set_post_callback(glad_post_call_callback);
#endif
    }

    void RunnerGlfwOpenGl3::Impl_SetupPlatformRendererBindings()
    {
        ImGui_ImplGlfw_InitForOpenGL(mWindow, true);
        ImGui_ImplOpenGL3_Init(Impl_GlslVersion().c_str());
    }

    bool RunnerGlfwOpenGl3::Impl_PollEvents()
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants
        // to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main
        // application. Generally you may always pass all inputs to dear imgui, and hide them from your
        // application based on those two flags.
        glfwPollEvents();
        bool exitRequired = glfwWindowShouldClose(mWindow);
        return exitRequired;
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
        // Impl_Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(mWindow);
        glfwTerminate();
    }

    void RunnerGlfwOpenGl3::Impl_SwapBuffers() { glfwSwapBuffers(mWindow); }

}  // namespace HelloImGui
#endif  // #ifdef HELLOIMGUI_USE_GLFW_OPENGL3
