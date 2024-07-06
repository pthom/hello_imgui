#ifdef HELLOIMGUI_USE_GLFW3
#include "opengl_setup_glfw.h"
#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/hello_imgui_include_opengl.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/backend_window_helper.h"

#include "GLFW/glfw3.h"


namespace HelloImGui { namespace BackendApi
{
#ifdef HELLOIMGUI_HAS_OPENGL3
    static int QueryMaxAntiAliasingSamples()
    {
        // First create a dummy window to make OpenGL context current
        GLFWwindow* dummyWindow = []
        {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            return glfwCreateWindow(64, 32, "Dummy", nullptr, nullptr);
        }();

        if (!dummyWindow)
            IM_ASSERT(false && "Failed to create temporary window.");
        glfwMakeContextCurrent(dummyWindow);

        // Init OpenGL loader
        OpenGlSetupGlfw openGlSetupGlfw;
        openGlSetupGlfw.InitGlLoader();

        GLint maxSamples;
        glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);

        glfwDestroyWindow(dummyWindow);

        return maxSamples;
    }

    static void ApplyAntiAliasingSamples(const OpenGlOptionsFilled_& openGlOptions)
    {
        int userQuerySamples = openGlOptions.AntiAliasingSamples;
        int maxGpuSamples = QueryMaxAntiAliasingSamples();
        int effectiveSamples = 8;

        if (effectiveSamples > maxGpuSamples)
            effectiveSamples = maxGpuSamples;

        if (userQuerySamples == 0)
            effectiveSamples = 0;
        else if (userQuerySamples > 0)
        {
            // if (userQuerySamples > maxGpuSamples)
            //    fprintf(stderr, "Warning: user requested %d samples, but GPU supports only %d samples. Using %d samples instead.\n", userQuerySamples, maxGpuSamples, maxGpuSamples);
            if (userQuerySamples <= maxGpuSamples)
                effectiveSamples = userQuerySamples;
        }

        if (effectiveSamples > 0)
        {
            #if ! defined(__EMSCRIPTEN__) && ! defined(HELLOIMGUI_USE_GLES3)  && ! defined(HELLOIMGUI_USE_GLES2)
            glEnable(GL_MULTISAMPLE); // multisampling is automatically enabled if the framebuffer supports it for OpenGL ES
            #endif
            glfwWindowHint(GLFW_SAMPLES, effectiveSamples);
        }
        else
        {
            glfwWindowHint(GLFW_SAMPLES, 0);
        }
    }
#endif  // #ifdef HELLOIMGUI_HAS_OPENGL3

    static void ApplyOpenGlOptions(const OpenGlOptionsFilled_& openGlOptions)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, openGlOptions.MajorVersion);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, openGlOptions.MinorVersion);
        if (openGlOptions.UseCoreProfile)
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        if (openGlOptions.UseForwardCompat)
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        #ifdef HELLOIMGUI_HAS_OPENGL3
            if (HelloImGui::GetRunnerParams()->rendererBackendType == RendererBackendType::OpenGL3)
                ApplyAntiAliasingSamples(openGlOptions);
        #endif
    }

    OpenGlOptionsFilled_ OpenGlSetupGlfw::Impl_MakeDefaultOpenGlOptionsForPlatform()
    {
        OpenGlOptionsFilled_ openGlOptions;

        openGlOptions.AntiAliasingSamples = 8;

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

    void OpenGlSetupGlfw::SelectOpenGlVersion(const OpenGlOptionsFilled_& options)
    {
        ApplyOpenGlOptions(options);
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
}} // namespace HelloImGui { namespace BackendApi

#endif // #ifdef HELLOIMGUI_USE_GLFW3
