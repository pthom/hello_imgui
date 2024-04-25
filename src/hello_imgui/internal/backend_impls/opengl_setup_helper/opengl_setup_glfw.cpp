#ifdef HELLOIMGUI_USE_GLFW3
#include "opengl_setup_glfw.h"
#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/hello_imgui_include_opengl.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/backend_window_helper.h"

#include "GLFW/glfw3.h"


namespace HelloImGui { namespace BackendApi
{

    static void ApplyOpenGlOptions(OpenGlOptions& openGlOptions)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, openGlOptions.MajorVersion);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, openGlOptions.MinorVersion);
        if (openGlOptions.UseCoreProfile)
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        if (openGlOptions.UseForwardCompat)
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    }

    static OpenGlOptions MakeOpenGlOptions()
    {
        auto* runnerParams = HelloImGui::GetRunnerParams();
        if (runnerParams->rendererBackendOptions.openGlOptions.has_value())
            return runnerParams->rendererBackendOptions.openGlOptions.value();

        OpenGlOptions openGlOptions;

        //
        // Compute MajorVersion, MinorVersion, UseCoreProfile, UseForwardCompat
        //
        openGlOptions.MajorVersion = 3;
        openGlOptions.MinorVersion = 3;
        openGlOptions.UseCoreProfile = true;
        openGlOptions.UseForwardCompat = true;

        #if defined(IMGUI_IMPL_OPENGL_ES3)
        {
            BACKEND_THROW("OpenGlSetupGlfw::SelectOpenGlVersion needs implementation for GLES !");
        }
        #elif defined(__APPLE__)
        {
            // 3.2+ only
            openGlOptions.MajorVersion = 3;
            openGlOptions.MinorVersion = 2;
            openGlOptions.UseCoreProfile = true;
            openGlOptions.UseForwardCompat = true;
        }
        #else
        {
            openGlOptions.MajorVersion = 3;
            openGlOptions.MinorVersion = 3;
            openGlOptions.UseCoreProfile = true;  // 3.2+ only
            openGlOptions.UseForwardCompat = true;  // 3.0+ only
        }
        #endif

        //
        // Compute GlslVersion
        //
        #if defined(IMGUI_IMPL_OPENGL_ES3)
            openGlOptions.GlslVersion = "300es";
        #elif defined(__APPLE__)
            openGlOptions.GlslVersion = "150";
        #else
            // GLSL 130
            openGlOptions.GlslVersion = "130";
        #endif

        return openGlOptions;
    }

    void OpenGlSetupGlfw::SelectOpenGlVersion()
    {
        OpenGlOptions openGlOptions = MakeOpenGlOptions();
        ApplyOpenGlOptions(openGlOptions);
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
        OpenGlOptions openGlOptions = MakeOpenGlOptions();
        return std::string("#version ") + openGlOptions.GlslVersion;
    }
}} // namespace HelloImGui { namespace BackendApi

#endif // #ifdef HELLOIMGUI_USE_GLFW3
