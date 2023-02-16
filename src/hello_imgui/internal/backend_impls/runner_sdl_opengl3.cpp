#ifdef HELLOIMGUI_USE_SDL_OPENGL3

#include "hello_imgui/hello_imgui_include_opengl.h"
#include "runner_sdl_opengl3.h"
#include "hello_imgui/hello_imgui_error.h"
#include "backend_window_helper/sdl_window_helper.h"
#include "internal/backend_impls/opengl_setup_helper/opengl_setup_sdl.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/win32_dpi_awareness.h"
#include "opengl_setup_helper/opengl_screenshot.h"
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl2.h>

#include <SDL.h>
#include <SDL_main.h>
#include <sstream>

namespace HelloImGui
{
    BackendApi::OpenGlSetupSdl gOpenGlSetupSdl;


    int HandleAppEvents(void *runnerSdlOpenGl3_void, SDL_Event *event)
    {
        RunnerSdlOpenGl3 * runnerSdlOpenGl3 = (RunnerSdlOpenGl3 *)(runnerSdlOpenGl3_void);
        if (runnerSdlOpenGl3->priv_HandleMobileDeviceEvent(event->type))
            return 0;
        else
            return 1;
    }

    RunnerSdlOpenGl3::RunnerSdlOpenGl3(RunnerParams & runnerParams) : AbstractRunner(runnerParams)
    {
        mBackendWindowHelper = std::make_unique<BackendApi::SdlWindowHelper>();
    }


    void RunnerSdlOpenGl3::Impl_InitBackend()
    {
        auto flags = SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER;
#ifdef __EMSCRIPTEN__
        flags = SDL_INIT_VIDEO | SDL_INIT_TIMER;
#endif
        if (SDL_Init(flags) != 0)
        {
            HIMG_ERROR(
                std::string("RunnerSdlOpenGl3::Impl_InitBackend error ")
                + SDL_GetError());
        }
        SDL_SetEventFilter(HandleAppEvents, this);
    }

    void RunnerSdlOpenGl3::Impl_Select_Gl_Version()
    {
        gOpenGlSetupSdl.SelectOpenGlVersion();
    }

    std::string RunnerSdlOpenGl3::Impl_GlslVersion()
    {
        return gOpenGlSetupSdl.GlslVersion();
    }

    void RunnerSdlOpenGl3::Impl_CreateWindow()
    {
        BackendApi::BackendOptions backendOptions;

        mWindow = mBackendWindowHelper->CreateWindow(params.appWindowParams, backendOptions);
        params.backendPointers.sdlWindow = mWindow;
    }

    void RunnerSdlOpenGl3::Impl_CreateGlContext()
    {
        mGlContext = SDL_GL_CreateContext((SDL_Window *)mWindow);
        IM_ASSERT(mGlContext != nullptr);

        SDL_GL_MakeCurrent((SDL_Window *)mWindow, mGlContext); // KK No
        SDL_GL_SetSwapInterval(1);  // Enable vsync
        params.backendPointers.sdlGlContext = mGlContext;
    }


    void RunnerSdlOpenGl3::Impl_InitGlLoader()
    {
        gOpenGlSetupSdl.InitGlLoader();
    }

    void RunnerSdlOpenGl3::Impl_SetupPlatformRendererBindings()
    {
        ImGui_ImplSDL2_InitForOpenGL((SDL_Window *)mWindow, mGlContext);
        ImGui_ImplOpenGL3_Init(Impl_GlslVersion().c_str());
    }

    void RunnerSdlOpenGl3::Impl_PollEvents()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (params.callbacks.AnyBackendEventCallback)
            {
                if (params.callbacks.AnyBackendEventCallback(&event))
                    continue;
            }
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                params.appShallExit = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID((SDL_Window *)mWindow))
            {
                params.appShallExit = true;
            }
        }
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
        SDL_DestroyWindow((SDL_Window *)mWindow);
        SDL_Quit();
    }

    void RunnerSdlOpenGl3::Impl_SwapBuffers() { SDL_GL_SwapWindow((SDL_Window *)mWindow); }



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

    ImageBuffer RunnerSdlOpenGl3::Impl_ScreenshotRgb()
    {
        return OpenglScreenshotRgb();
    }

}  // namespace HelloImGui

#endif  // #ifdef HELLOIMGUI_USE_SDL_OPENGL3
