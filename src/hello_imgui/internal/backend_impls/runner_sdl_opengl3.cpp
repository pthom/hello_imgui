#ifdef HELLOIMGUI_USE_SDL_OPENGL3

#include "hello_imgui/hello_imgui_include_opengl.h"
#include "runner_sdl_opengl3.h"
#include "hello_imgui/hello_imgui_error.h"
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl.h>

#include <SDL.h>
#include <SDL_main.h>
#include <sstream>

namespace HelloImGui
{
    int HandleAppEvents(void *runnerSdlOpenGl3_void, SDL_Event *event)
    {
        RunnerSdlOpenGl3 * runnerSdlOpenGl3 = (RunnerSdlOpenGl3 *)(runnerSdlOpenGl3_void);
        if (runnerSdlOpenGl3->priv_HandleMobileDeviceEvent(event->type))
            return 0;
        else
            return 1;
    }


    void RunnerSdlOpenGl3::Impl_InitBackend()
    {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
        {
            HIMG_THROW_STRING(
                std::string("RunnerSdlOpenGl3::Impl_InitBackend error ")
                + SDL_GetError());
        }
        SDL_SetEventFilter(HandleAppEvents, this);
    }

    void RunnerSdlOpenGl3::Impl_Select_Gl_Version()
    {
#if defined(__EMSCRIPTEN__)
        /*
        {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        }*/
#elif defined(HELLOIMGUI_USE_GLES3)
        {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                                SDL_GL_CONTEXT_PROFILE_ES);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        }
#elif defined(HELLOIMGUI_USE_GLES2)
        {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 2);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                                SDL_GL_CONTEXT_PROFILE_ES);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        }
#elif defined(__APPLE__)
        {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);  // Always required on Mac
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        }
#else
        {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        }
#endif
    }

    std::string RunnerSdlOpenGl3::Impl_GlslVersion()
    {
#if defined(HELLOIMGUI_USE_GLES3)
        const char* glsl_version = "#version 300 es";
#elif defined(HELLOIMGUI_USE_GLES2)
        const char* glsl_version = "#version 200 es";
#elif defined(__APPLE__)
        const char* glsl_version = "#version 150";
#else
        const char* glsl_version = "#version 130";
#endif
        return glsl_version;
    }

    void RunnerSdlOpenGl3::Impl_CreateWindowAndContext()
    {
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

#ifndef __EMSCRIPTEN__
        const auto &backendWindowParams = params.appWindowParams;
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
        if (!mWindow)
            HIMG_THROW("RunnerSdlOpenGl3::Impl_CreateWindowAndContext : SDL_CreateWindow returned NULL");

        if (backendWindowParams.fullScreen)
        {
            SDL_SetWindowFullscreen(mWindow, SDL_WINDOW_FULLSCREEN);
        }
#else
        const auto &backendWindowParams = params.appWindowParams;
        SDL_DisplayMode current;
        SDL_GetCurrentDisplayMode(0, &current);
        SDL_WindowFlags window_flags =
            (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        mWindow = SDL_CreateWindow(
            backendWindowParams.windowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
#endif

        mGlContext = SDL_GL_CreateContext(mWindow);
        if (!mGlContext)
            HIMG_THROW("RunnerSdlOpenGl3::Impl_CreateWindowAndContext(): Failed to initialize WebGL context!");

        SDL_GL_MakeCurrent(mWindow, mGlContext); // KK No
        SDL_GL_SetSwapInterval(1);  // Enable vsync

        params.backendPointers.sdlWindow = mWindow;
        params.backendPointers.sdlGlContext = mGlContext;
    }

    void RunnerSdlOpenGl3::Impl_InitGlLoader()
    {
        // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_ES3) || defined(IMGUI_IMPL_OPENGL_ES2) || defined(__EMSCRIPTEN__)
        ; // nothing to do
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
        bool err = gl3wInit() != 0;
        if (err)
            HIMG_THROW("Failed to initialize OpenGL loader!");
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
        bool err = glewInit() != GLEW_OK;
        if (err)
            HIMG_THROW("Failed to initialize OpenGL loader!");
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
        bool err = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress)) == 0;
        if (err)
            HIMG_THROW("Failed to initialize OpenGL loader!");
#endif
    }

    void RunnerSdlOpenGl3::Impl_SetupPlatformRendererBindings()
    {
        ImGui_ImplSDL2_InitForOpenGL(mWindow, mGlContext);
        ImGui_ImplOpenGL3_Init(Impl_GlslVersion().c_str());
    }

    bool RunnerSdlOpenGl3::Impl_PollEvents()
    {
#ifdef HELLOIMGUI_USE_POWERSAVE
        ImGui_ImplSDL2_WaitForEvent(mWindow);
#endif

        SDL_Event event;
        bool exitRequired = false;
        while (SDL_PollEvent(&event))
        {
            if (params.callbacks.AnyBackendEventCallback)
            {
                if (params.callbacks.AnyBackendEventCallback(&event))
                    continue;
            }
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

    void RunnerSdlOpenGl3::Impl_NewFrame_3D() { ImGui_ImplOpenGL3_NewFrame(); }

    void RunnerSdlOpenGl3::Impl_NewFrame_Backend() { ImGui_ImplSDL2_NewFrame(); }

    void RunnerSdlOpenGl3::Impl_Frame_3D_ClearColor()
    {
        auto& io = ImGui::GetIO();
        glViewport(0, 0, static_cast<int>(io.DisplaySize.x), static_cast<int>(io.DisplaySize.y));
        ImVec4 clear_color = params.imGuiWindowParams.backgroundColor;
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void RunnerSdlOpenGl3::Impl_RenderDrawData_To_3D() { ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); }

    void RunnerSdlOpenGl3::Impl_UpdateAndRenderAdditionalPlatformWindows()
    {
        SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }

    void RunnerSdlOpenGl3::Impl_Cleanup()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        SDL_GL_DeleteContext(mGlContext);
        SDL_DestroyWindow(mWindow);
        SDL_Quit();
    }

    void RunnerSdlOpenGl3::Impl_SwapBuffers() { SDL_GL_SwapWindow(mWindow); }



    bool RunnerSdlOpenGl3::priv_HandleMobileDeviceEvent(unsigned int sdl_EventType)
    {
#ifdef HELLOIMGUI_MOBILEDEVICE
        switch(sdl_EventType)
        {
            case SDL_APP_TERMINATING:
                /* Terminate the app.
                   Shut everything down before returning from this function.
                */
                OnDestroy();
                return true;
            case SDL_APP_LOWMEMORY:
                /* You will get this when your app is paused and iOS wants more memory.
                   Release as much memory as possible.
                */
                OnLowMemory();
                return true;
            case SDL_APP_WILLENTERBACKGROUND:
                /* Prepare your app to go into the background.  Stop loops, etc.
                   This gets called when the user hits the home button, or gets a call.
                */
                OnPause();
                return true;
            case SDL_APP_DIDENTERBACKGROUND:
                /* This will get called if the user accepted whatever sent your app to the background.
                   If the user got a phone call and canceled it, you'll instead get an    SDL_APP_DIDENTERFOREGROUND event and restart your loops.
                   When you get this, you have 5 seconds to save all your state or the app will be terminated.
                   Your app is NOT active at this point.
                */
                OnPause();
                return true;
            case SDL_APP_WILLENTERFOREGROUND:
                /* This call happens when your app is coming back to the foreground.
                    Restore all your state here.
                */
                OnResume();
                return true;
            case SDL_APP_DIDENTERFOREGROUND:
                /* Restart your loops here.
                   Your app is interactive and getting CPU again.
                */
                OnResume();
                return true;
            default:
                /* No special processing, add it to the event queue */
                return false;
        }
#else // #ifdef HELLOIMGUI_MOBILEDEVICE
      return false;
#endif
    }

    void RunnerSdlOpenGl3screenshot()
    {

    }


}  // namespace HelloImGui

#endif  // #ifdef HELLOIMGUI_USE_SDL_OPENGL3
