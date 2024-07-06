#ifdef HELLOIMGUI_USE_SDL2
#include "runner_sdl2.h"

#ifdef HELLOIMGUI_HAS_OPENGL
#include "imgui_impl_opengl3.h"
#include "rendering_opengl3.h"
#endif
#ifdef HELLOIMGUI_HAS_METAL
#include "rendering_metal.h"
#endif
#ifdef HELLOIMGUI_HAS_VULKAN
#include "rendering_vulkan.h"
#endif
#ifdef HELLOIMGUI_HAS_DIRECTX11
#include "rendering_dx11.h"
#endif
#ifdef HELLOIMGUI_HAS_DIRECTX12
#include "rendering_dx12.h"
#endif


#include "hello_imgui/hello_imgui_assets.h"
#include "hello_imgui/internal/backend_impls/rendering_callbacks.h"
#include "stb_image.h"
#include "imgui_impl_sdl2.h"

#include "hello_imgui/hello_imgui_error.h"
#include "backend_window_helper/sdl_window_helper.h"
#include "hello_imgui/internal/backend_impls/opengl_setup_helper/opengl_setup_sdl.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/win32_dpi_awareness.h"

#include <SDL.h>
#include <SDL_main.h>
#include <sstream>
#include <functional>

namespace HelloImGui
{
    BackendApi::OpenGlSetupSdl gOpenGlSetupSdl;


    int HandleAppEvents(void *runnerSdlOpenGl3_void, SDL_Event *event)
    {
        RunnerSdl2 * runnerSdlOpenGl3 = (RunnerSdl2 *)(runnerSdlOpenGl3_void);
        if (runnerSdlOpenGl3->priv_HandleMobileDeviceEvent(event->type))
            return 0;
        else
            return 1;
    }

    RunnerSdl2::RunnerSdl2(RunnerParams & runnerParams) : AbstractRunner(runnerParams)
    {
        mBackendWindowHelper = std::make_unique<BackendApi::SdlWindowHelper>();
    }


    void RunnerSdl2::Impl_InitPlatformBackend()
    {
        auto flags = SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER;
#ifdef __EMSCRIPTEN__
        flags = SDL_INIT_VIDEO | SDL_INIT_TIMER;
#endif
        if (SDL_Init(flags) != 0)
        {
            HIMG_ERROR(
                std::string("RunnerSdlOpenGl3::Impl_InitPlatformBackend error ")
                + SDL_GetError());
        }

        // Enable native IME
        SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

        SDL_SetEventFilter(HandleAppEvents, this);
    }

    void RunnerSdl2::Impl_CreateWindow(std::function<void()> renderCallbackDuringResize)
    {
        BackendApi::BackendOptions backendOptions;
        backendOptions.rendererBackendType = params.rendererBackendType;
        mWindow = mBackendWindowHelper->CreateWindow(params.appWindowParams, backendOptions, renderCallbackDuringResize);
        params.backendPointers.sdlWindow = mWindow;
    }

    void RunnerSdl2::Impl_PollEvents()
    {
        SDL_Event event;
        while    (SDL_PollEvent(&event))
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

    void RunnerSdl2::Impl_UpdateAndRenderAdditionalPlatformWindows()
    {
        #ifdef HELLOIMGUI_HAS_OPENGL
            SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        #endif
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        #ifdef HELLOIMGUI_HAS_OPENGL
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        #endif
    }

    void RunnerSdl2::Impl_Cleanup()
    {
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        SDL_GL_DeleteContext(mGlContext);
        SDL_DestroyWindow((SDL_Window *)mWindow);
        SDL_Quit();
    }

    void RunnerSdl2::Impl_SwapBuffers()
    {
        // Call of RenderingCallbacks_Impl_SwapBuffers
        #ifdef HELLOIMGUI_HAS_OPENGL
            if (params.rendererBackendType == RendererBackendType::OpenGL3)
                SDL_GL_SwapWindow((SDL_Window *)mWindow);
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
            if (params.rendererBackendType == RendererBackendType::DirectX12)
                SwapDx12Buffers();
        #endif
    }


    bool RunnerSdl2::priv_HandleMobileDeviceEvent(unsigned int sdl_EventType)
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

    void RunnerSdl2::Impl_SetWindowIcon()
    {
        std::string iconFile = "app_settings/icon.png";
        if (!HelloImGui::AssetExists(iconFile))
            return;

        auto imageAsset = HelloImGui::LoadAssetFileData(iconFile.c_str());
        int width, height, channels;
        unsigned char *image = stbi_load_from_memory(
            (stbi_uc *)imageAsset.data, (int)imageAsset.dataSize, &width, &height, &channels, 4);  // force RGBA channels

        if (image)
        {
            // Create an SDL_Surface from the raw pixel data
            SDL_Surface *iconSurface = SDL_CreateRGBSurfaceFrom(
                image, width, height, 32, 4 * width, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

            if (iconSurface)
            {
                SDL_SetWindowIcon((SDL_Window *)mWindow, iconSurface);
                SDL_FreeSurface(iconSurface);  // SDL_CreateRGBSurfaceFrom does not free the pixel data
            }
            else
                HIMG_LOG("RunnerSdlOpenGl3::Impl_SetWindowIcon: Failed to create surface");

            stbi_image_free(image);
        }
        else
            HIMG_LOG("RunnerSdlOpenGl3::Impl_SetWindowIcon: Failed to load window icon: " + iconFile);
        HelloImGui::FreeAssetFileData(&imageAsset);
    }

    void RunnerSdl2::Impl_NewFrame_PlatformBackend() { ImGui_ImplSDL2_NewFrame(); }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Link to Rendering Backends (OpenGL, Vulkan, ...)
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef HELLOIMGUI_HAS_OPENGL
    void Impl_LinkPlatformAndRenderBackends_SdlOpenGl(const RunnerSdl2& runner)
    {
        ImGui_ImplSDL2_InitForOpenGL((SDL_Window *)runner.mWindow, runner.mGlContext);
        ImGui_ImplOpenGL3_Init(runner.Impl_GlslVersion().c_str());
    }

    void RunnerSdl2::Impl_CreateGlContext()
    {
        mGlContext = SDL_GL_CreateContext((SDL_Window *)mWindow);
        IM_ASSERT(mGlContext != nullptr);

        SDL_GL_MakeCurrent((SDL_Window *)mWindow, mGlContext); // KK No
        SDL_GL_SetSwapInterval(1);  // Enable vsync
        params.backendPointers.sdlGlContext = mGlContext;
    }
    void RunnerSdl2::Impl_InitGlLoader() { gOpenGlSetupSdl.InitGlLoader(); }
    void RunnerSdl2::Impl_Select_Gl_Version()
    {
        auto openGlOptions = gOpenGlSetupSdl.OpenGlOptionsWithUserSettings();
        gOpenGlSetupSdl.SelectOpenGlVersion(openGlOptions);
    }

    std::string RunnerSdl2::Impl_GlslVersion() const
    {
        return std::string("#version ") + gOpenGlSetupSdl.OpenGlOptionsWithUserSettings().GlslVersion;
    }
#endif // HELLOIMGUI_HAS_OPENGL

#ifdef HELLOIMGUI_HAS_METAL
    void Impl_LinkPlatformAndRenderBackends_SdlMetal(const RunnerSdl2& runner)
    {
        PrepareSdlForMetal((SDL_Window*)runner.mWindow, runner.params.rendererBackendOptions);
    }
#endif
#ifdef HELLOIMGUI_HAS_VULKAN
    void Impl_LinkPlatformAndRenderBackends_SdlVulkan(const RunnerSdl2& runner)
    {
        PrepareSdlForVulkan((SDL_Window*)runner.mWindow);
    }
#endif
#ifdef HELLOIMGUI_HAS_DIRECTX11
    void Impl_LinkPlatformAndRenderBackends_SdlDirectX11(const RunnerSdl2& runner)
    {
        PrepareSdlForDx11((SDL_Window*)runner.mWindow);
    }
#endif
#ifdef HELLOIMGUI_HAS_DIRECTX12
    void Impl_LinkPlatformAndRenderBackends_SdlDirectX12(const RunnerSdl2& runner)
    {
        PrepareSdlForDx12((SDL_Window*)runner.mWindow);
    }
#endif

    void RunnerSdl2::Impl_LinkPlatformAndRenderBackends()
    {
        auto& self = *this;
        if (params.rendererBackendType == HelloImGui::RendererBackendType::OpenGL3)
        {
            #ifdef HELLOIMGUI_HAS_OPENGL
                Impl_LinkPlatformAndRenderBackends_SdlOpenGl(self);
            #else
                IM_ASSERT(false && "OpenGL3 backend not available");
            #endif
        }
        else if (params.rendererBackendType == HelloImGui::RendererBackendType::Metal)
        {
            #ifdef HELLOIMGUI_HAS_METAL
                Impl_LinkPlatformAndRenderBackends_SdlMetal(self);
            #else
                IM_ASSERT(false && "Metal backend not available");
            #endif
        }
        else if (params.rendererBackendType == HelloImGui::RendererBackendType::Vulkan)
        {
            #ifdef HELLOIMGUI_HAS_VULKAN
                Impl_LinkPlatformAndRenderBackends_SdlVulkan(self);
            #else
                IM_ASSERT(false && "Vulkan backend not available");
            #endif
        }
        else if (params.rendererBackendType == HelloImGui::RendererBackendType::DirectX11)
        {
            #ifdef HELLOIMGUI_HAS_DIRECTX11
                Impl_LinkPlatformAndRenderBackends_SdlDirectX11(self);
            #else
                IM_ASSERT(false && "DirectX11 backend not available");
            #endif
        }
        else if (params.rendererBackendType == HelloImGui::RendererBackendType::DirectX12)
        {
            #ifdef HELLOIMGUI_HAS_DIRECTX12
                Impl_LinkPlatformAndRenderBackends_SdlDirectX12(self);
            #else
                IM_ASSERT(false && "DirectX12 backend not available");
            #endif
        }
    }

}  // namespace HelloImGui

#endif  // #ifdef HELLOIMGUI_USE_SDL2
