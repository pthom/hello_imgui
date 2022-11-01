#ifdef HELLOIMGUI_USE_GLFW
#include "opengl_setup_glfw.h"
#include "hello_imgui/hello_imgui_include_opengl.h"
#include "internal/backend_impls/backend_window_helper/backend_window_helper.h"

#include "GLFW/glfw3.h"


namespace HelloImGui { namespace BackendApi
{

    void OpenGlSetupGlfw::SelectOpenGlVersion()
    {
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
#if defined(IMGUI_IMPL_OPENGL_ES3)
        const char* glsl_version = "#version 300es";
#elif defined(__APPLE__)
        const char* glsl_version = "#version 150";
#else
        // GLSL 130
        const char* glsl_version = "#version 130";
#endif
        return glsl_version;
    }
}} // namespace HelloImGui { namespace BackendApi

#endif // #ifdef HELLOIMGUI_USE_GLFW
