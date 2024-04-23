#ifdef HELLOIMGUI_USE_SDL2

#include "opengl_setup_sdl.h"
#include "hello_imgui/hello_imgui_include_opengl.h"
#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/backend_window_helper.h"

#include "SDL.h"


namespace HelloImGui { namespace BackendApi
{
    void OpenGlSetupSdl::SelectOpenGlVersion()
    {
        int major = 3;
        int minor = 3;
        bool useCoreProfile = false;
        bool useForwardCompat = false;

#if defined(__EMSCRIPTEN__)
        /*
        {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        }*/
#elif defined(IMGUI_IMPL_OPENGL_ES3)
        {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
            major = 3;
            minor = 0;
        }
#elif defined(IMGUI_IMPL_OPENGL_ES2)
        {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 2);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
            major = 2;
            minor = 0;
        }
#elif defined(__APPLE__)
        {
            useForwardCompat= true; // Always required on Mac
            useCoreProfile = true;
            major = 3;
            minor = 2;
        }
#else
        {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
            useCoreProfile = true;
            major = 3;
            minor = 3;
        }
#endif

#ifndef __EMSCRIPTEN__
        // Gl version not set in Emscripten, for legacy reasons that are not clear
        // (was not used in HelloImGui before April 2024)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
#endif
        if (useCoreProfile)
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        if (useForwardCompat)
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
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
        std::string glsl_version;
#if defined(IMGUI_IMPL_OPENGL_ES3)
        glsl_version = "#version 300 es";
#elif defined(IMGUI_IMPL_OPENGL_ES2)
        glsl_version = "#version 200 es";
#elif defined(__APPLE__)
        glsl_version = "#version 150";
#else
        glsl_version = "#version 130";
#endif

        auto runnerParams = HelloImGui::GetRunnerParams();
        if (runnerParams->rendererBackendOptions.openGlOptions.has_value())
        {
            auto& openGlOptions = runnerParams->rendererBackendOptions.openGlOptions.value();
            glsl_version = openGlOptions.GlslVersion;
        }

        return glsl_version.c_str();
    }
}} // namespace HelloImGui { namespace BackendApi

#endif // #ifdef HELLOIMGUI_USE_SDL2
