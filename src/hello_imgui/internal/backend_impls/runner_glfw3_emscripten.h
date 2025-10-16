#pragma once
//#define __EMSCRIPTEN__
#if defined(__EMSCRIPTEN__) && defined(HELLOIMGUI_USE_GLFW3)

#include "hello_imgui/internal/backend_impls/runner_glfw3.h"

namespace HelloImGui
{
    class RunnerGlfw3Emscripten : public RunnerGlfw3
    {
       public:
        RunnerGlfw3Emscripten(RunnerParams & runnerParams) : RunnerGlfw3(runnerParams) {}
        virtual ~RunnerGlfw3Emscripten() = default;
        void Run() override;

       protected:
        void Impl_Select_Gl_Version() override;
        std::string Impl_GlslVersion() const override;
        void Impl_InitGlLoader() override;
    };

}  // namespace HelloImGui

#endif  // #if defined(__EMSCRIPTEN__) && defined(HELLOIMGUI_USE_GLFW3)
