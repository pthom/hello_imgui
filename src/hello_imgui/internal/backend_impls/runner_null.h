#pragma once
#include "hello_imgui/internal/backend_impls/abstract_runner.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/null_window_helper.h"
#include "hello_imgui/internal/backend_impls/null_config.h"


namespace HelloImGui
{
    class RunnerNull : public AbstractRunner
    {
       public:
        RunnerNull(RunnerParams & runnerParams): AbstractRunner(runnerParams) {
            mBackendWindowHelper = std::make_unique<BackendApi::NullWindowHelper>();
        }
        virtual ~RunnerNull() = default;

       protected:
        //
        // Methods related to the platform backend (SDL, Glfw, ...)
        //
        void Impl_InitPlatformBackend() override {
            auto size = NullConfig::GetScreenBounds().size;
            ImGui::GetIO().DisplaySize = ImVec2((float)size[0], (float)size[1]);
        }
        void Impl_CreateWindow(std::function<void()> renderCallbackDuringResize) override {}
        void Impl_PollEvents() override {}
        void Impl_NewFrame_PlatformBackend() override {}
        void Impl_UpdateAndRenderAdditionalPlatformWindows() override {}
        void Impl_Cleanup() override {}
        void Impl_SwapBuffers() override {}
        void Impl_SetWindowIcon() override {}
        void Impl_LinkPlatformAndRenderBackends() override {}

#ifdef HELLOIMGUI_HAS_OPENGL
       public:
        void Impl_Select_Gl_Version() override {}
        void Impl_InitGlLoader() override {}
        std::string Impl_GlslVersion() const override { return "";}
        void Impl_CreateGlContext()  override {}
#endif

    };

}  // namespace HelloImGui
