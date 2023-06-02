#pragma once
#include "hello_imgui/runner_params.h"
#include "hello_imgui/hello_imgui_screenshot.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/backend_window_helper.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/window_geometry_helper.h"

#include <memory>

namespace HelloImGui
{

class AbstractRunner
{
   public:
    AbstractRunner(RunnerParams &params_);
    virtual ~AbstractRunner() = default;

    RunnerParams & params;

    virtual void Run();

    void Setup();
    void CreateFramesAndRender();
    void RenderGui();
    void TearDown(bool gotException);

   public:
    // Events for mobile devices
    void OnPause();
    void OnResume();
    void OnDestroy();
    void OnLowMemory();

    // For jupyter notebook, which displays a screenshot post execution
    ImageBuffer ScreenshotRgb() { return Impl_ScreenshotRgb(); }

    //
    // Dpi related methods
    //
    // DpiWindowSizeFactor() is the factor by which window size should be multiplied to get a similar visible size on different OSes.
    // It returns ApplicationScreenPixelPerInch / 96  under windows and linux. Under macOS, it will return 1.
    float DpiWindowSizeFactor();
    // If we want a font to visually render like a font size of 14 we need to multiply its size by this factor
    float DpiFontLoadingFactor();
    // returns the default value that should be stored inside `ImGui::GetIO().FontGlobalScale`
    float ImGuiDefaultFontGlobalScale();

    void LayoutSettings_SwitchLayout(const std::string& layoutName);

    void        SaveUserPref(const std::string& userPrefName, const std::string& userPrefContent);
    std::string LoadUserPref(const std::string& userPrefName);

protected:
    friend std::string GlslVersion();

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

    virtual void Impl_PollEvents() = 0;
    virtual void Impl_NewFrame_3D() = 0;
    virtual void Impl_NewFrame_Backend() = 0;
    virtual void Impl_Frame_3D_ClearColor() = 0;
    virtual void Impl_RenderDrawData_To_3D() = 0;
    virtual void Impl_UpdateAndRenderAdditionalPlatformWindows() = 0;
    virtual void Impl_SwapBuffers() = 0;
    virtual void Impl_Cleanup() = 0;

    virtual ImageBuffer Impl_ScreenshotRgb() { return ImageBuffer{}; }

private:
    void PrepareWindowGeometry();
    void HandleDpiOnSecondFrame();
    void ReloadFontIfFailed();
    void MakeWindowSizeRelativeTo96Ppi_IfRequired();
    bool ShallSizeWindowRelativeTo96Ppi();
    bool WantAutoSize();
    // Logic for idling
    void IdleBySleeping();
    bool ShallIdleThisFrame_Emscripten();
    std::string IniPartsFilename();

    void SetLayoutResetIfNeeded();

    void LayoutSettings_HandleChanges();
    void LayoutSettings_Load();
    void LayoutSettings_Save();

protected:
    BackendApi::WindowPointer mWindow = nullptr;
    std::unique_ptr<BackendApi::IBackendWindowHelper> mBackendWindowHelper;
private:
    std::unique_ptr<WindowGeometryHelper> mGeometryHelper;
    bool mPotentialFontLoadingError = false;
    int mIdxFrame = 0;
    bool mWasWindowAutoResizedOnPreviousFrame = false;

};


}  // namespace HelloImGui
