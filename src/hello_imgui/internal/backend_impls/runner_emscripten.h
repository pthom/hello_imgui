#pragma once
#ifdef __EMSCRIPTEN__

#include "hello_imgui/internal/backend_impls/runner_sdl2.h"

namespace HelloImGui
{
    class RunnerEmscripten : public RunnerSdl2
    {
       public:
        RunnerEmscripten(RunnerParams & runnerParams) : RunnerSdl2(runnerParams) {}
        virtual ~RunnerEmscripten() = default;
        void Run() override;

       protected:
        void Impl_Select_Gl_Version() override;
        std::string Impl_GlslVersion() override;
        void Impl_InitGlLoader() override;
    };

}  // namespace HelloImGui

#endif  // #ifdef __EMSCRIPTEN__
