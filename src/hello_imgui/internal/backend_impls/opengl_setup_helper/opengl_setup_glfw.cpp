#ifdef HELLOIMGUI_USE_GLFW3
#include "opengl_setup_glfw.h"
#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/hello_imgui_include_opengl.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/backend_window_helper.h"

#include "GLFW/glfw3.h"


namespace HelloImGui { namespace BackendApi
{

    void OpenGlSetupGlfw::SelectOpenGlVersion()
    {
        int major = 3;
        int minor = 3;
        bool useCoreProfile = true;
        bool useForwardCompat = true;

        #if defined(IMGUI_IMPL_OPENGL_ES3)
        {
            BACKEND_THROW("OpenGlSetupGlfw::SelectOpenGlVersion needs implementation for GLES !");
            //            SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 3);
            //            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
            //                                SDL_GL_CONTEXT_PROFILE_ES);
            //            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            //            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        }
        #elif defined(__APPLE__)
        {
            // 3.2+ only
            major = 3;
            minor = 2;
            useCoreProfile = true;
            useForwardCompat = true;
        }
        #else
        {
            major = 3;
            minor = 3;
            useCoreProfile = true;  // 3.2+ only
            useForwardCompat = true;  // 3.0+ only
        }
        #endif

        auto* runnerParams = HelloImGui::GetRunnerParams();
        if (runnerParams->rendererBackendOptions.openGlOptions.has_value())
        {
            auto& openGlOptions = runnerParams->rendererBackendOptions.openGlOptions.value();
            major = openGlOptions.MajorVersion;
            minor = openGlOptions.MinorVersion;
            useCoreProfile = openGlOptions.UseCoreProfile;
            useForwardCompat = openGlOptions.UseForwardCompat;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
        if (useCoreProfile)
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        if (useForwardCompat)
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    }

    void OpenGlSetupGlfw::InitGlLoader()
    {
        #ifndef __EMSCRIPTEN__
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
            BACKEND_THROW("RunnerGlfwOpenGl3::Impl_InitGlLoader(): Failed to initialize OpenGL loader!");
        }
        #endif  // #ifndef __EMSCRIPTEN__
    }

    std::string OpenGlSetupGlfw::GlslVersion()
    {
        std::string glsl_version;
        #if defined(IMGUI_IMPL_OPENGL_ES3)
            glsl_version = "#version 300es";
        #elif defined(__APPLE__)
            glsl_version = "#version 150";
        #else
            // GLSL 130
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

#endif // #ifdef HELLOIMGUI_USE_GLFW3
