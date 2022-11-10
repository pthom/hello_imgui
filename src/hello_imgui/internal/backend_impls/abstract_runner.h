#pragma once
#include "hello_imgui/runner_params.h"
#include "hello_imgui/hello_imgui_screenshot.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/backend_window_helper.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/window_geometry_helper.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/window_autosize_helper.h"

#include <memory>

namespace HelloImGui
{

class AbstractRunner
{
   public:
    /// Step 0: Construct a concrete Runner (for example RunnerSdlOpenGl3 or RunnerGlfwOpenGl3)
    AbstractRunner(RunnerParams &params_);
    virtual ~AbstractRunner() = default;

    RunnerParams & params;

    /// Step 4.a: Call Run()
    virtual void Run();

    /// Step 4.b: Or implement your own Run using Setup()/Render()/TearDown()
    void Setup();
    void CreateFramesAndRender(int idxFrame);
    void RenderGui(int idxFrame);
    void TearDown();

   public:
    // Events for mobile devices
    void OnPause();
    void OnResume();
    void OnDestroy();
    void OnLowMemory();

    ImageBuffer ScreenshotRgb() { return Impl_ScreenshotRgb(); }

   protected:
    //
    // The methods Impl_* are astract
    // and shall be overriden in the concrete implementations by derivates
    //
    virtual void Impl_InitBackend() = 0;
    virtual void Impl_Select_Gl_Version() = 0;
    virtual std::string Impl_GlslVersion() = 0;

    virtual void Impl_CreateWindow() = 0;
    virtual void Impl_CreateGlContext() = 0;
    virtual void Impl_InitGlLoader() = 0;
    virtual void Impl_SetupPlatformRendererBindings() = 0;
    virtual void Impl_SetupImgGuiContext() {}

    virtual bool Impl_PollEvents() = 0;  // returns true if exit required
    virtual void Impl_NewFrame_3D() = 0;
    virtual void Impl_NewFrame_Backend() = 0;
    virtual void Impl_Frame_3D_ClearColor() = 0;
    virtual void Impl_RenderDrawData_To_3D() = 0;
    virtual void Impl_UpdateAndRenderAdditionalPlatformWindows() = 0;
    virtual void Impl_SwapBuffers() = 0;
    virtual void Impl_Cleanup() = 0;

    virtual ImageBuffer Impl_ScreenshotRgb() { return ImageBuffer{}; }

private:
    void PrepareAutoSize();
    void ForceWindowPositionOrSize();

protected:
    BackendApi::WindowPointer mWindow = nullptr;
    std::unique_ptr<BackendApi::IBackendWindowHelper> mBackendWindowHelper;
private:
    std::unique_ptr<WindowGeometryHelper> mGeometryHelper;
    std::unique_ptr<WindowAutoSizeHelper> mAutoSizeHelper;
};

}  // namespace HelloImGui
