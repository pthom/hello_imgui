#pragma once
#ifdef HELLOIMGUI_USE_QT
#include "hello_imgui/internal/backend_impls/abstract_runner.h"
#include <memory>

namespace HelloImGui
{
class RunnerQt_WindowImpl;

class RunnerQt : public AbstractRunner
{
   public:
    RunnerQt(RunnerParams& runnerParams);
    virtual ~RunnerQt();
    void Run() override;

   private:
    void RunQGuiApplication();

   protected:
    void Impl_InitPlatformBackend() override;
    void Impl_Select_Gl_Version() override;
    std::string Impl_GlslVersion() override;
    void Impl_CreateWindow() override;
    void Impl_CreateGlContext() override;
    void Impl_InitGlLoader() override;
    void Impl_SetupPlatformRendererBindings() override;

    void Impl_PollEvents() override;
    void Impl_NewFrame_3D() override;
    void Impl_NewFrame_PlatformBackend() override;
    void Impl_Frame_3D_ClearColor() override;
    void Impl_RenderDrawData_To_3D() override;
    void Impl_UpdateAndRenderAdditionalPlatformWindows() override;
    void Impl_Cleanup() override;
    void Impl_SwapBuffers() override;

   private:
    std::unique_ptr<RunnerQt_WindowImpl> impl_;
};

}  // namespace HelloImGui

#endif  // #ifdef HELLOIMGUI_USE_QT
