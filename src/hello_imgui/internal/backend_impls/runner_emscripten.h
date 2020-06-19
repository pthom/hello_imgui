#pragma once
#ifdef __EMSCRIPTEN__

#include "hello_imgui/internal/backend_impls/runner_sdl_opengl3.h"

namespace HelloImGui
{
    class RunnerEmscripten : public RunnerSdlOpenGl3
    {
       public:
        RunnerEmscripten(RunnerParams & runnerParams) : RunnerSdlOpenGl3(runnerParams) {}
        virtual ~RunnerEmscripten() = default;
        void Run() override;

       protected:
        void Impl_Select_Gl_Version() override;
        std::string Impl_GlslVersion() override;
        void Impl_InitGlLoader() override;
        void Impl_SetupImgGuiContext() override;
    };

}  // namespace HelloImGui

#endif  // #ifdef __EMSCRIPTEN__
