#ifdef HELLOIMGUI_USE_SDL2

#include "opengl_setup_sdl.h"
#include "hello_imgui/hello_imgui_include_opengl.h"
#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/backend_window_helper.h"

#include "SDL.h"


namespace HelloImGui { namespace BackendApi
{
    static bool gUseConservativeSettings = false;

    static void ApplyOpenGlOptions(OpenGlOptions& openGlOptions)
    {
        #ifndef __EMSCRIPTEN__
            // Gl version not set in Emscripten, for legacy reasons that are not clear
            // (was not used in HelloImGui before April 2024)
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, openGlOptions.MajorVersion);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, openGlOptions.MinorVersion);
        #endif
        if (openGlOptions.UseCoreProfile)
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        if (openGlOptions.UseForwardCompat)
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    }

    static OpenGlOptions MakeOpenGlOptions()
    {
        // Notes:
        // - SelectOpenGlVersion will first try non-conservative settings, and if it fails, try conservative settings
        // - if runnerParams->rendererBackendOptions.openGlOptions.has_value(), we always use it

        auto* runnerParams = HelloImGui::GetRunnerParams();
        if (runnerParams->rendererBackendOptions.openGlOptions.has_value())
            return runnerParams->rendererBackendOptions.openGlOptions.value();

        OpenGlOptions openGlOptions;

        //
        // Compute MajorVersion, MinorVersion, UseCoreProfile, UseForwardCompat
        //
        openGlOptions.MajorVersion = 3;
        openGlOptions.MinorVersion = 3;
        openGlOptions.UseCoreProfile = false;
        openGlOptions.UseForwardCompat = false;

        #if defined(__EMSCRIPTEN__)
            // Nothing!
        #elif defined(IMGUI_IMPL_OPENGL_ES3)
        {
            openGlOptions.MajorVersion = 3;
            openGlOptions.MinorVersion = 0;
        }
        #elif defined(IMGUI_IMPL_OPENGL_ES2)
        {
            openGlOptions.MajorVersion = 2;
            openGlOptions.MinorVersion = 0;
        }
        #elif defined(__APPLE__)
        {
            openGlOptions.MajorVersion = 3;
            openGlOptions.MinorVersion = 2;
            openGlOptions.UseCoreProfile = true;
            openGlOptions.UseForwardCompat = true; // Always required on Mac
        }
        #else
        {
            openGlOptions.MajorVersion = 3;
            openGlOptions.MinorVersion = 3;
            openGlOptions.UseCoreProfile = true;
        }
        #endif

        //
        // Compute GlslVersion
        //
        #if defined(IMGUI_IMPL_OPENGL_ES3)
            openGlOptions.GlslVersion = "#version 300 es";
        #elif defined(IMGUI_IMPL_OPENGL_ES2)
            openGlOptions.GlslVersion = "#version 200 es";
        #elif defined(__APPLE__)
            openGlOptions.GlslVersion = "#version 150";
        #else
            openGlOptions.GlslVersion = "#version 130";
        #endif

        //
        // Conservative settings based on user feedback
        //
        if (gUseConservativeSettings)
        {
            #ifdef _WIN32
            // cf https://github.com/pthom/imgui_bundle/issues/206#issuecomment-2074578423
            openGlOptions.MajorVersion = 3;
            openGlOptions.MinorVersion = 1;
            openGlOptions.UseCoreProfile = false;
            openGlOptions.UseForwardCompat = true;
            openGlOptions.GlslVersion = "#version 130";
            #endif
        }

        return openGlOptions;
    }

    static bool CanCreateWindowWithCurrentOpenGlSettings()
    {
        bool success = false;
        SDL_Window *dummyWindow = SDL_CreateWindow("test", 0, 0, 20, 20, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        if (dummyWindow)
        {
            SDL_GLContext dummyContext = SDL_GL_CreateContext(dummyWindow);
            if (dummyContext)
            {
                SDL_GL_DeleteContext(dummyContext);
                success = true;
            }
            SDL_DestroyWindow(dummyWindow);
        }
        return success;
    }

    static void AdditionalOpenGlPreSetup()
    {
        #if defined(__EMSCRIPTEN__)
            // Nothing!
        #elif defined(IMGUI_IMPL_OPENGL_ES3)
        {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        }
        #elif defined(IMGUI_IMPL_OPENGL_ES2)
        {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 2);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        }
        #elif defined(__APPLE__)
            // Nothing!
        #else
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        #endif

    }

    static void AdditionalOpenGlPostSetup()
    {
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    }


    void OpenGlSetupSdl::SelectOpenGlVersion()
    {
        AdditionalOpenGlPreSetup();
        OpenGlOptions openGlOptions = MakeOpenGlOptions();
        ApplyOpenGlOptions(openGlOptions);
        AdditionalOpenGlPostSetup();

        // Test if we should use more conservative settings.
        // For the moment, we only test conservative settings under Windows,
        // since this is the only platform for which we have user feedback about that.
        {
            #ifdef _WIN32
            bool success = CanCreateWindowWithCurrentOpenGlSettings();

            if (!success)
            {
                printf("Can't create window with standard OpenGL settings. Trying more conservative settings.\n");
                gUseConservativeSettings = true;

                AdditionalOpenGlPreSetup();
                OpenGlOptions openGlOptions = MakeOpenGlOptions();
                ApplyOpenGlOptions(openGlOptions);
                AdditionalOpenGlPostSetup();

                success = CanCreateWindowWithCurrentOpenGlSettings();
                IM_ASSERT(success && "OpenGlSetupSdl::SelectOpenGlVersion(): Can't create window with conservative OpenGL settings.");
            }
            #endif
        }
    }

    void OpenGlSetupSdl::InitGlLoader()
    {
        // Initialize OpenGL loader
        #if defined(IMGUI_IMPL_OPENGL_ES3) || defined(IMGUI_IMPL_OPENGL_ES2) || defined(__EMSCRIPTEN__)
            ; // nothing to do
        #elif defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
            bool err = gl3wInit() != 0;
            if (err)
                BACKEND_THROW("Failed to initialize OpenGL loader!");
        #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
            bool err = glewInit() != GLEW_OK;
            if (err)
                BACKEND_THROW("Failed to initialize OpenGL loader!");
        #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
            bool err = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress)) == 0;
            if (err)
                BACKEND_THROW("Failed to initialize OpenGL loader!");
        #endif
    }

    std::string OpenGlSetupSdl::GlslVersion()
    {
        OpenGlOptions openGlOptions = MakeOpenGlOptions();
        return openGlOptions.GlslVersion;
    }
}} // namespace HelloImGui { namespace BackendApi

#endif // #ifdef HELLOIMGUI_USE_SDL2
