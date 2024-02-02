#pragma once
#ifdef __EMSCRIPTEN__

#include "hello_imgui/internal/backend_impls/runner_sdl2.h"

namespace HelloImGui
{
    class RunnerSdlEmscripten : public RunnerSdl2
    {
       public:
        RunnerSdlEmscripten(RunnerParams & runnerParams) : RunnerSdl2(runnerParams) {}
        virtual ~RunnerSdlEmscripten() = default;
        void Run() override;

       protected:
        void Impl_Select_Gl_Version() override;
        std::string Impl_GlslVersion() const override;
        void Impl_InitGlLoader() override;
    };

}  // namespace HelloImGui

#endif  // #ifdef __EMSCRIPTEN__
