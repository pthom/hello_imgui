#pragma once
#include "hello_imgui/hello_imgui_error.h"
#include "hello_imgui/hello_imgui_screenshot.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/backend_window_helper.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/window_geometry_helper.h"
#include "hello_imgui/internal/backend_impls/rendering_callbacks.h"
#include "hello_imgui/runner_params.h"

#include <memory>
#include <functional>

namespace HelloImGui
{
// For more info, see "Anatomy of an ImGui app lifecycle (cf ImGui examples)", in rendering_callbacks.h
class AbstractRunner
{
   public:
    explicit AbstractRunner(RunnerParams &params_);
    virtual ~AbstractRunner() = default;

    RunnerParams & params;

    virtual void Run(); // Only overriden in emscripten

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
    ImageBuffer ScreenshotRgb() { return mRenderingBackendCallbacks->Impl_ScreenshotRgb_3D(); }

    //
    // Dpi related methods
    //
    // DpiWindowSizeFactor() is the factor by which window size should be multiplied to get a similar visible size on different OSes.
    // It returns ApplicationScreenPixelPerInch / 96  under windows and linux. Under macOS, it will return 1.
    float DpiWindowSizeFactor();
    // If we want a font to visually render like a font size of 14 we need to multiply its size by this factor
    float DpiFontLoadingFactor();
    // returns the default value that should be stored inside `ImGui::GetIO().FontGlobalScale`
    static float ImGuiDefaultFontGlobalScale();

    void LayoutSettings_SwitchLayout(const std::string& layoutName);

    void        SaveUserPref(const std::string& userPrefName, const std::string& userPrefContent);
    std::string LoadUserPref(const std::string& userPrefName);

protected:
    friend std::string GlslVersion();

    //
    // The methods Impl_* are abstract
    // and shall be overriden in the concrete implementations by derived classes
    //

    //
    // Methods related to the platform backend (SDL, Glfw, ...)
    //
    virtual void Impl_InitPlatformBackend() = 0;
    virtual void Impl_CreateWindow() = 0;
    virtual void Impl_PollEvents() = 0;
    virtual void Impl_NewFrame_PlatformBackend() = 0;
    virtual void Impl_UpdateAndRenderAdditionalPlatformWindows() = 0;
    virtual void Impl_SwapBuffers() = 0;
    virtual void Impl_Cleanup() = 0;
    virtual void Impl_SetWindowIcon() {}

    //
    // Linking the platform backend (SDL, Glfw, ...) to the rendering backend (OpenGL, ...)
    //
    virtual void Impl_LinkPlatformAndRenderBackends() = 0;
    // Specific to OpenGL
    #ifdef HELLOIMGUI_HAS_OPENGL
        virtual void Impl_Select_Gl_Version() = 0;
        virtual void Impl_InitGlLoader() = 0;
        virtual std::string Impl_GlslVersion() = 0;
        virtual void Impl_CreateGlContext() = 0;
    #endif

    //
    // Methods and callbacks related to the rendering backend (OpenGL, ...)
    //
    // Wish should be filled by Impl_InitRenderBackendCallbacks
    virtual void Impl_InitRenderBackendCallbacks() = 0;
    RenderingCallbacksPtr mRenderingBackendCallbacks;

private:
    void InitImGuiContext();
    void SetImGuiPrefs();

    void PrepareWindowGeometry();
    void HandleDpiOnSecondFrame();
    void ReloadFontIfFailed() const;
    void MakeWindowSizeRelativeTo96Ppi_IfRequired();
    bool ShallSizeWindowRelativeTo96Ppi();
    bool WantAutoSize();
    // Logic for idling
    void IdleBySleeping();
    bool ShallIdleThisFrame_Emscripten();

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
