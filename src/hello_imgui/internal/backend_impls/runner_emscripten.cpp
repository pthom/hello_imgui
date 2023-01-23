#include "runner_emscripten.h"
#include <iostream>
#ifdef __EMSCRIPTEN__

#include <emscripten.h>

namespace HelloImGui
{
    RunnerEmscripten* gRunnerEmscripten = nullptr;

    void emscripten_imgui_main_loop(void* arg)
    {
        (void) arg;
        gRunnerEmscripten->CreateFramesAndRender();
    }

    void RunnerEmscripten::Run()
    {
        gRunnerEmscripten = this;
        gRunnerEmscripten->Setup();

        emscripten_cancel_main_loop();

        SDL_GL_SetSwapInterval(1);  // Enable vsync

        // This function call won't return, and will engage in an infinite loop,
        // processing events from the browser, and dispatching them.
        // int fps = 0; // 0 <=> let the browser decide. This is the recommended way, see
        // https://emscripten.org/docs/api_reference/emscripten.h.html#browser-execution-environment
        emscripten_set_main_loop_arg(emscripten_imgui_main_loop, NULL, params.emscripten_fps, true);
    }

    void RunnerEmscripten::Impl_SetupImgGuiContext()
    {
        AbstractRunner::Impl_SetupImgGuiContext();
        // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the
        // imgui.ini file. You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
        ImGui::GetIO().IniFilename = NULL;
    }

    void RunnerEmscripten::Impl_Select_Gl_Version()
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    }

    std::string RunnerEmscripten::Impl_GlslVersion()
    {
//        const char* glsl_version = "#version 300 es";
        const char* glsl_version = "#version 100";
        return glsl_version;
    }

    void RunnerEmscripten::Impl_InitGlLoader() {}

}  // namespace HelloImGui

#endif  // #ifdef __EMSCRIPTEN__
