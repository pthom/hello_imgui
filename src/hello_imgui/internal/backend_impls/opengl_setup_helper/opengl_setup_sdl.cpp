#ifdef HELLOIMGUI_USE_SDL2

#include "opengl_setup_sdl.h"
#include "hello_imgui/hello_imgui_include_opengl.h"
#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/backend_window_helper.h"

#include "SDL.h"


namespace HelloImGui { namespace BackendApi
{
#ifdef HELLOIMGUI_HAS_OPENGL3

    static int QueryMaxAntiAliasingSamples()
    {
        // First create a dummy window to make OpenGL context current
        SDL_Window* dummyWindow = SDL_CreateWindow(
            "Dummy",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            64, 32,
            SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN
        );

        if (!dummyWindow)
        {
            IM_ASSERT(false && "Failed to create temporary window.");
            return 0;
        }

        SDL_GLContext dummyContext = SDL_GL_CreateContext(dummyWindow);
        if (!dummyContext)
        {
            SDL_DestroyWindow(dummyWindow);
            IM_ASSERT(false && "Failed to create temporary GL context.");
            return 0;
        }

        // Init OpenGL loader
        OpenGlSetupSdl openGlSetupSdl;
        openGlSetupSdl.InitGlLoader();

        GLint maxSamples;
        glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);

        SDL_GL_DeleteContext(dummyContext);
        SDL_DestroyWindow(dummyWindow);

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
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, effectiveSamples);
            #if ! defined(__EMSCRIPTEN__) && ! defined(HELLOIMGUI_USE_GLES3)  && ! defined(HELLOIMGUI_USE_GLES2)
            glEnable(GL_MULTISAMPLE); // multisampling is automatically enabled if the framebuffer supports it for OpenGL ES
            #endif
        }
        else
        {
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
        }
    }
#endif  // #ifdef HELLOIMGUI_HAS_OPENGL3

    static void ApplyOpenGlOptions(const OpenGlOptionsFilled_& openGlOptions)
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

        #ifdef HELLOIMGUI_HAS_OPENGL3
            if (HelloImGui::GetRunnerParams()->rendererBackendType == RendererBackendType::OpenGL3)
                ApplyAntiAliasingSamples(openGlOptions);
        #endif
    }

    OpenGlOptionsFilled_ OpenGlSetupSdl::Impl_MakeDefaultOpenGlOptionsForPlatform()
    {
        OpenGlOptionsFilled_ openGlOptions;

        openGlOptions.AntiAliasingSamples = 8;

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
            openGlOptions.GlslVersion = "300 es";
        #elif defined(IMGUI_IMPL_OPENGL_ES2)
            openGlOptions.GlslVersion = "200 es";
        #elif defined(__APPLE__)
            openGlOptions.GlslVersion = "150";
        #else
            openGlOptions.GlslVersion = "130";
        #endif

        return openGlOptions;
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


    void OpenGlSetupSdl::SelectOpenGlVersion(const OpenGlOptionsFilled_& options)
    {
        AdditionalOpenGlPreSetup();
        ApplyOpenGlOptions(options);
        AdditionalOpenGlPostSetup();
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
}} // namespace HelloImGui { namespace BackendApi

#endif // #ifdef HELLOIMGUI_USE_SDL2
