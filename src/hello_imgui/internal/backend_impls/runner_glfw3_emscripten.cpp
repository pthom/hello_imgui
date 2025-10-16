#include "runner_glfw3_emscripten.h"
#if defined(__EMSCRIPTEN__) && defined(HELLOIMGUI_USE_GLFW3)
#include <iostream>

#include <emscripten.h>

namespace HelloImGui
{
    RunnerGlfw3Emscripten* gRunnerGlfw3Emscripten = nullptr;

    void emscripten_imgui_main_loop_glfw3(void* arg)
    {
        (void) arg;
        gRunnerGlfw3Emscripten->CreateFramesAndRender();
    }

    void RunnerGlfw3Emscripten::Run()
    {
        #if defined(HELLOIMGUI_WITH_TEST_ENGINE) && !defined(HELLOIMGUI_EMSCRIPTEN_PTHREAD)
            printf("RunnerGlfw3Emscripten::Run Disabling useImGuiTestEngine since compiled without pthread\n");
            params.useImGuiTestEngine = false;
        #endif

        gRunnerGlfw3Emscripten = this;
        gRunnerGlfw3Emscripten->Setup();

        emscripten_cancel_main_loop();

        //SDL_GL_SetSwapInterval(1);  // Enable vsync
        glfwSwapInterval(1);  // Enable vsync

        // This function call won't return, and will engage in an infinite loop,
        // processing events from the browser, and dispatching them.
        // int fps = 0; // 0 <=> let the browser decide. This is the recommended way, see
        // https://emscripten.org/docs/api_reference/emscripten.h.html#browser-execution-environment
        emscripten_set_main_loop_arg(emscripten_imgui_main_loop_glfw3, NULL, params.emscripten_fps, true);
    }

    void RunnerGlfw3Emscripten::Impl_Select_Gl_Version()
    {
        // SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        // SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    }

    std::string RunnerGlfw3Emscripten::Impl_GlslVersion() const
    {
        const char* glsl_version = "#version 300 es";
        //const char* glsl_version = "#version 100";
        return glsl_version;
    }

    void RunnerGlfw3Emscripten::Impl_InitGlLoader() {}

}  // namespace HelloImGui

#endif  // #if defined(__EMSCRIPTEN__) && defined(HELLOIMGUI_USE_GLFW3)
