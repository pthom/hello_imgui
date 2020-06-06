#ifdef HELLOIMGUI_USE_SDL

#ifdef HELLOIMGUI_USE_GLAD
#include <glad/glad.h>
#endif
#include "runner_sdl.h"
#include <examples/imgui_impl_opengl3.h>
#include <examples/imgui_impl_sdl.h>

#include <SDL.h>
#include <sstream>

namespace HelloImGui
{
    void RunnerSdl::Impl_InitBackend()
    {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
        {
            std::stringstream msg;
            msg << "RunnerSdl::Impl_InitBackend error " << SDL_GetError();
            throw std::runtime_error(msg.str().c_str());
        }
    }

    void RunnerSdl::Impl_Select_Gl_Version()
    {
        // Decide GL+GLSL versions
#if __APPLE__
        // GL 3.2 Core + GLSL 150
        // const char* glsl_version = "#version 150";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);  // Always required on Mac
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
        // GL 3.0 + GLSL 130
        // const char* glsl_version = "#version 130";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif
    }

    std::string RunnerSdl::Impl_GlslVersion()
    {
#if __APPLE__
        // GL 3.2 Core + GLSL 150
        const char* glsl_version = "#version 150";
#else
        // GL 3.0 + GLSL 130
        const char* glsl_version = "#version 130";
#endif
        return glsl_version;
    }

    void RunnerSdl::Impl_CreateWindowAndContext()
    {
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

#ifndef __EMSCRIPTEN__
        const auto &backendWindowParams = params.backendWindowParams;
        ImVec2 windowPosition = backendWindowParams.windowPosition;
        ImVec2 windowSize = backendWindowParams.windowSize;
        SDL_WindowFlags window_flags =
            (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        int xPos = SDL_WINDOWPOS_CENTERED, yPos = SDL_WINDOWPOS_CENTERED;
        if (windowPosition.x >= -10000.f)
        {
            xPos = static_cast<int>(windowPosition.x);
            yPos = static_cast<int>(windowPosition.y);
        }
        mWindow = SDL_CreateWindow(backendWindowParams.windowTitle.c_str(),
                                   xPos,
                                   yPos,
                                   static_cast<int>(windowSize.x),
                                   static_cast<int>(windowSize.y),
                                   window_flags);

        if (backendWindowParams.fullScreen)
        {
            SDL_SetWindowFullscreen(mWindow, SDL_WINDOW_FULLSCREEN);
        }
#else
        SDL_DisplayMode current;
        SDL_GetCurrentDisplayMode(0, &current);
        SDL_WindowFlags window_flags =
            (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        mWindow = SDL_CreateWindow(
            mBackendWindowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
#endif

        mGlContext = SDL_GL_CreateContext(mWindow);
        if (!mGlContext)
            throw std::runtime_error("RunnerSdl::Impl_CreateWindowAndContext(): Failed to initialize WebGL context!");

        SDL_GL_MakeCurrent(mWindow, mGlContext);
#ifndef __EMSCRIPTEN__
        SDL_GL_SetSwapInterval(1);  // Enable vsync
#endif
    }

    void RunnerSdl::Impl_InitGlLoader()
    {
#ifndef __EMSCRIPTEN__
        // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
        bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
        bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
        bool err = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress)) == 0;
#else
        bool err = false;  // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of
                           // initialization.
#endif
        if (err)
        {
            throw std::runtime_error("Failed to initialize OpenGL loader!");
        }
#endif  // #ifndef __EMSCRIPTEN__

#ifdef GLAD_DEBUG
        glad_set_pre_callback(glad_pre_call_callback);
        glad_set_post_callback(glad_post_call_callback);
#endif
    }

    void RunnerSdl::Impl_SetupPlatformRendererBindings()
    {
        ImGui_ImplSDL2_InitForOpenGL(mWindow, mGlContext);
        ImGui_ImplOpenGL3_Init(Impl_GlslVersion().c_str());
    }

    bool RunnerSdl::Impl_PollEvents()
    {
        SDL_Event event;
        bool exitRequired = false;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                exitRequired = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(mWindow))
            {
                exitRequired = true;
            }
        }
        return exitRequired;
    }

    void RunnerSdl::Impl_NewFrame_3D() { ImGui_ImplOpenGL3_NewFrame(); }

    void RunnerSdl::Impl_NewFrame_Backend() { ImGui_ImplSDL2_NewFrame(mWindow); }

    void RunnerSdl::Impl_Frame_3D_ClearColor()
    {
        auto& io = ImGui::GetIO();
        glViewport(0, 0, static_cast<int>(io.DisplaySize.x), static_cast<int>(io.DisplaySize.y));
        ImVec4 clear_color = params.imGuiWindowParams.BackgroundColor;
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void RunnerSdl::Impl_RenderDrawData_To_3D() { ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); }

    void RunnerSdl::Impl_UpdateAndRenderAdditionalPlatformWindows()
    {
        SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }

    void RunnerSdl::Impl_Cleanup()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        SDL_GL_DeleteContext(mGlContext);
        SDL_DestroyWindow(mWindow);
        SDL_Quit();
    }

    void RunnerSdl::Impl_SwapBuffers() { SDL_GL_SwapWindow(mWindow); }

}  // namespace HelloImGui

#endif  // #ifdef HELLOIMGUI_USE_SDL