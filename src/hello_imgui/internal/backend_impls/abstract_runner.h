#pragma once
#include "hello_imgui/hello_imgui_error.h"
#include "hello_imgui/hello_imgui_screenshot.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/backend_window_helper.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/window_geometry_helper.h"
#include "hello_imgui/internal/backend_impls/rendering_callbacks.h"
#include "hello_imgui/internal/backend_impls/remote_display_handler.h"
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
    virtual ~AbstractRunner();

    RunnerParams & params;

    virtual void Run(); // Only overriden in emscripten

    void Setup();
    void CreateFramesAndRender(bool insideReentrantCall = false);
    void RenderGui();
    void TearDown(bool gotException);

    // Events for mobile devices
    void OnPause();
    void OnResume();
    void OnDestroy();
    void OnLowMemory();

    // For jupyter notebook, which displays a screenshot post execution
    ImageBuffer ScreenshotRgb() { return mRenderingBackendCallbacks->Impl_ScreenshotRgb_3D(); }

    void ChangeWindowSize(ScreenSize windowSize);

    void LayoutSettings_SwitchLayout(const std::string& layoutName);
    bool ShouldRemoteDisplay();


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
    virtual void Impl_CreateWindow(std::function<void()> renderCallbackDuringResize) = 0;
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
    public:
        virtual void Impl_Select_Gl_Version() = 0;
        virtual void Impl_InitGlLoader() = 0;
        virtual std::string Impl_GlslVersion() const = 0;
        virtual void Impl_CreateGlContext() = 0;
    #endif

private:
    void InitImGuiContext();
    void SetImGuiPrefs();
    void InitRenderBackendCallbacks();

    void SetupDpiAwareParams();
    bool CheckDpiAwareParamsChanges();
    void PrepareWindowGeometry();
    void AdjustWindowBoundsAfterCreation_IfDpiChangedBetweenRuns();
    void HandleDpiOnSecondFrame();
    void MakeWindowSizeRelativeTo96Ppi_IfRequired();
    bool ShallSizeWindowRelativeTo96Ppi();
    bool WantAutoSize();

    void SetLayoutResetIfNeeded();

    void LayoutSettings_HandleChanges();
    void LayoutSettings_Load();
    void LayoutSettings_Save();

public:
    BackendApi::WindowPointer mWindow = nullptr;
protected:
    std::unique_ptr<BackendApi::IBackendWindowHelper> mBackendWindowHelper;
private:
    std::unique_ptr<WindowGeometryHelper> mGeometryHelper;
    bool mPotentialFontLoadingError = false;
    int mIdxFrame = 0;
    bool mWasWindowAutoResizedOnPreviousFrame = false;

    // Differentiate between cases where the window was resized by code
    // and cases where the window was resized by the user
    // (in which we have a gotcha, because PollEvents() will *block*
    // until the user releases the mouse button)
    bool mWasWindowResizedByCodeDuringThisFrame = false;
    std::function<void()> setWasWindowResizedByCodeDuringThisFrame =
        [&]() { mWasWindowResizedByCodeDuringThisFrame = true; };

    // Callbacks related to the rendering backend (OpenGL, ...)
    RenderingCallbacksPtr mRenderingBackendCallbacks;

    RemoteDisplayHandler mRemoteDisplayHandler;
};


}  // namespace HelloImGui
