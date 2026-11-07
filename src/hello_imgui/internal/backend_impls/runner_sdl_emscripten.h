#pragma once
#if defined(__EMSCRIPTEN__) && defined(HELLOIMGUI_USE_SDL2)

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
        void Impl_InitPlatformBackend() override;  // sets up the touch input (ImGuiTouch::SetupEmscripten), the tap zones
        void Impl_PollEvents() override;  // pushes the tap zones, updates the keyboard, then polls
        void Impl_Select_Gl_Version() override;
        std::string Impl_GlslVersion() const override;
        void Impl_InitGlLoader() override;
    };

}  // namespace HelloImGui

#endif  // #if defined(__EMSCRIPTEN__) && defined(HELLOIMGUI_USE_SDL2)
