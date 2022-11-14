#include "hello_imgui/internal/backend_impls/abstract_runner.h"
#include "hello_imgui/internal/docking_details.h"
#include "hello_imgui/internal/menu_statusbar.h"
#include "hello_imgui/image_from_asset.h"
#include "hello_imgui/hello_imgui_theme.h"
#include "imgui.h"

#include "hello_imgui/internal/imgui_global_context.h" // must be included before imgui_internal.h
#include "imgui_internal.h"


namespace HelloImGui
{
// Encapsulated inside hello_imgui_screenshot.cpp
void setFinalAppWindowScreenshotRgbBuffer(const ImageBuffer& b);


AbstractRunner::AbstractRunner(RunnerParams &params_)
    : params(params_)
{};

void AbstractRunner::Run()
{
    Setup();

    auto originalStyle = ImGui::GetStyle();
    int frameIdx = 0;
#ifdef HELLOIMGUI_MOBILEDEVICE
    while (true)
    {
        CreateFramesAndRender(frameIdx);
    }
#else
    try
    {
        while (!params.appShallExit)
        {
            if (frameIdx == 1)
                ForceWindowPositionOrSize();

            CreateFramesAndRender(frameIdx);

            frameIdx += 1;
        }

        // Store screenshot before exiting
        {
            ImageBuffer b = ScreenshotRgb();
            setFinalAppWindowScreenshotRgbBuffer(b);
        }

        if (params.appWindowParams.restorePreviousGeometry)
            mGeometryHelper->WriteLastRunWindowBounds(mBackendWindowHelper->GetWindowBounds(mWindow));
        TearDown();
    }
    catch(std::exception&)
    {
        // Late handling of user exceptions: TearDown backend and rethrow
        TearDown();
        throw;
    }
#endif
}

#ifdef HELLO_IMGUI_IMGUI_SHARED
static void*   MyMallocWrapper(size_t size, void* user_data)    { IM_UNUSED(user_data); return malloc(size); }
static void    MyFreeWrapper(void* ptr, void* user_data)        { IM_UNUSED(user_data); free(ptr); }
#endif

void AbstractRunner::PrepareAutoSize()
{
    std::optional<ScreenSize> yetUnknownRealWindowSizeAfterAutoSize = std::nullopt;
    mGeometryHelper = std::make_unique<WindowGeometryHelper>(params.appWindowParams.windowGeometry, params.appWindowParams.restorePreviousGeometry);
    mAutoSizeHelper = std::make_unique<WindowAutoSizeHelper>(*mGeometryHelper);
    auto windowBounds = mGeometryHelper->AppWindowBoundsInitial(mBackendWindowHelper->GetMonitorsWorkAreas(), yetUnknownRealWindowSizeAfterAutoSize);
    if (params.appWindowParams.restorePreviousGeometry && mGeometryHelper->ReadLastRunWindowBounds().has_value())
        params.appWindowParams.windowGeometry.positionMode = WindowPositionMode::FromCoords;
    params.appWindowParams.windowGeometry.position = windowBounds.position;
    params.appWindowParams.windowGeometry.size = windowBounds.size;
}

void AbstractRunner::ForceWindowPositionOrSize()
{
    // This is done at the second frame, once we know the size of all the widgets
    if (mAutoSizeHelper->WantAutoSize())
    {
        // The window was resized by mAutoSizeHelper
        // We should now recenter the window if needed and ensure it fits on the monitor
        auto realWindowSizeAfterAutoSize = mBackendWindowHelper->GetWindowBounds(mWindow).size;
        auto windowBounds = mGeometryHelper->AppWindowBoundsInitial(
            mBackendWindowHelper->GetMonitorsWorkAreas(), realWindowSizeAfterAutoSize);
        mBackendWindowHelper->SetWindowBounds(mWindow, windowBounds);
    }
    /*
        if not self.window_geometry_helper.want_autosize() and not self.imgui_app_params.app_window_full_screen:
            # The window was not resized
            # However, we forcefully set its position once again, since some backends ignore
            # it position at window creation time (SDL)
            real_window_size_after_auto_size = self.backend.get_window_size()
            window_bounds = self.window_geometry_helper.app_window_bounds_initial(
                self.all_monitors_work_areas, real_window_size_after_auto_size)
            self.backend.set_window_position(window_bounds.window_position)
     */
    mAutoSizeHelper->EnsureWindowFitsMonitor(mBackendWindowHelper.get(), mWindow);
}


void AbstractRunner::Setup()
{
    Impl_InitBackend();
    Impl_Select_Gl_Version();

    PrepareAutoSize();
    Impl_CreateWindow();

    Impl_CreateGlContext();
    Impl_InitGlLoader();
    IMGUI_CHECKVERSION();

#ifdef HELLO_IMGUI_IMGUI_SHARED
    auto ctx = ImGui::CreateContext();
    GImGui = ctx;
    ImGui::SetCurrentContext(ctx);
    ImGui::SetAllocatorFunctions(MyMallocWrapper, MyFreeWrapper);
#else
    ImGui::CreateContext();
#endif

    // High DPI handling on windows & linux
    // cf https://github.com/pthom/imgui_bundle/issues/7
    {
        float scaleFactor = mBackendWindowHelper->GetWindowDpiScaleFactor(mWindow);
        params.appWindowParams.outWindowDpiFactor = scaleFactor;
        if (scaleFactor > 1.f)
        {
            ImGuiStyle& style = ImGui::GetStyle();
            style.ScaleAllSizes(scaleFactor);
        }
    }

    Impl_SetupImgGuiContext();
    params.callbacks.SetupImGuiConfig();
    if (params.imGuiWindowParams.enableViewports)
    {
#ifndef __EMSCRIPTEN__
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif
    }
    params.callbacks.SetupImGuiStyle();
    Impl_SetupPlatformRendererBindings();
    ImGui::GetIO().Fonts->Clear();
    params.callbacks.LoadAdditionalFonts();
    ImGui::GetIO().Fonts->Build();
    DockingDetails::ConfigureImGuiDocking(params.imGuiWindowParams);

    ImGuiTheme::ApplyTweakedTheme(params.imGuiWindowParams.tweakedTheme);

    if (params.callbacks.PostInit)
        params.callbacks.PostInit();

}

void AbstractRunner::RenderGui(int idxFrame)
{
    DockingDetails::ProvideWindowOrDock(params.imGuiWindowParams, params.dockingParams);

    if (params.imGuiWindowParams.showMenuBar)
        Menu_StatusBar::ShowMenu(params);

    if (params.callbacks.ShowGui)
    {
        if (idxFrame == 0)
            mAutoSizeHelper->BeginMeasureSize(mBackendWindowHelper.get(), mWindow);

        params.callbacks.ShowGui();

        if (idxFrame == 0)
            mAutoSizeHelper->EndMeasureSize(mBackendWindowHelper.get(), mWindow);
    }

    DockingDetails::ShowDockableWindows(params.dockingParams.dockableWindows);

    if (params.imGuiWindowParams.showStatusBar)
        Menu_StatusBar::ShowStatusBar(params);

    Theme_WindowGui(params.imGuiWindowParams.tweakedTheme);

    DockingDetails::CloseWindowOrDock(params.imGuiWindowParams);
}


void AbstractRunner::CreateFramesAndRender(int idxFrame)
{
    assert(params.fpsIdle >= 0.f);
    if (params.fpsIdle > 0.f)
    {
        double waitTimeout = 1. / (double) params.fpsIdle;
        mBackendWindowHelper->WaitForEventTimeout(waitTimeout);
    }

    if (Impl_PollEvents())
        params.appShallExit = true;

    Impl_NewFrame_3D();
    Impl_NewFrame_Backend();
    ImGui::NewFrame();

    RenderGui(idxFrame);

    ImGui::Render();
    Impl_Frame_3D_ClearColor();
    Impl_RenderDrawData_To_3D();

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        Impl_UpdateAndRenderAdditionalPlatformWindows();

    Impl_SwapBuffers();
}

void AbstractRunner::OnPause()
{
#ifdef HELLOIMGUI_MOBILEDEVICE
    if (params.callbacks.mobileCallbacks.OnPause)
        params.callbacks.mobileCallbacks.OnPause();
#endif
}

void AbstractRunner::OnResume()
{
#ifdef HELLOIMGUI_MOBILEDEVICE
    if (params.callbacks.mobileCallbacks.OnResume)
        params.callbacks.mobileCallbacks.OnResume();
#endif
}

void AbstractRunner::OnDestroy()
{
#ifdef HELLOIMGUI_MOBILEDEVICE
    if (params.callbacks.mobileCallbacks.OnDestroy)
        params.callbacks.mobileCallbacks.OnDestroy();
#endif
}

void AbstractRunner::OnLowMemory()
{
#ifdef HELLOIMGUI_MOBILEDEVICE
    if (params.callbacks.mobileCallbacks.OnLowMemory)
        params.callbacks.mobileCallbacks.OnLowMemory();
#endif
}

void AbstractRunner::TearDown()
{
    HelloImGui::internal::Free_ImageFromAssetMap();

    if (params.callbacks.BeforeExit)
        params.callbacks.BeforeExit();
    Impl_Cleanup();
}

}  // namespace HelloImGui
