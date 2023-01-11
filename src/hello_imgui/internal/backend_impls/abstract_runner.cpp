#include "hello_imgui/internal/backend_impls/abstract_runner.h"
#include "hello_imgui/internal/docking_details.h"
#include "hello_imgui/internal/menu_statusbar.h"
#include "hello_imgui/image_from_asset.h"
#include "hello_imgui/hello_imgui_theme.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/window_autosize_helper.h"
#include "imgui.h"

#include "hello_imgui/internal/imgui_global_context.h" // must be included before imgui_internal.h
#include "imgui_internal.h"

#include <chrono>
#include <cassert>


namespace
{
    class ClockSeconds_
    {
        // Typical C++ shamanic incantations to get a time in seconds
    private:
        using Clock = std::chrono::high_resolution_clock;
        using second = std::chrono::duration<double, std::ratio<1>>;
        std::chrono::time_point<Clock> mStart;

    public:
        ClockSeconds_() : mStart(Clock::now()) {}

        double elapsed() const
        {
            return std::chrono::duration_cast<second>
                (Clock::now() - mStart).count();
        }
    };

    double ClockSeconds()
    {
        static ClockSeconds_ watch;
        return watch.elapsed();
    }

}


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
                UpdateWindowGeometryOnSecondFrame();

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

void AbstractRunner::FinishAutoSize_IfRequired()
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
}

bool AbstractRunner::ShallSizeWindowRelativeTo96Ppi() 
{
    bool shallSizeRelativeTo96Ppi;
    {
        bool doRestorePreviousGeometry = (params.appWindowParams.restorePreviousGeometry &&
                                          mGeometryHelper->ReadLastRunWindowBounds().has_value());

        bool isWindowPpiRelativeSize = (params.appWindowParams.windowGeometry.windowSizeMeasureMode ==
                                        HelloImGui::WindowSizeMeasureMode::RelativeTo96Ppi);

        bool isStandardSizeMode = params.appWindowParams.windowGeometry.windowSizeState == HelloImGui::WindowSizeState::Standard;

        shallSizeRelativeTo96Ppi =
            isStandardSizeMode && isWindowPpiRelativeSize && !doRestorePreviousGeometry;
    }
    return shallSizeRelativeTo96Ppi;
}


void AbstractRunner::MakeWindowSizeRelativeTo96Ppi_IfRequired()
{
    if (ShallSizeWindowRelativeTo96Ppi())
    {
        float scaleFactor = mBackendWindowHelper->GetWindowDpiScaleFactor(mWindow);
        if (scaleFactor != 1.f)
        {
            auto bounds = mBackendWindowHelper->GetWindowBounds(mWindow);

            // update size
            bounds.size = {(int)((float)bounds.size[0] * scaleFactor),
                           (int)((float)bounds.size[1] * scaleFactor)};

            // update position
            if (   (params.appWindowParams.windowGeometry.positionMode == HelloImGui::WindowPositionMode::MonitorCenter)
                || (params.appWindowParams.windowGeometry.positionMode == HelloImGui::WindowPositionMode::OsDefault))
            {
                WindowAutoSizeHelper helper(*mGeometryHelper);
                auto monitorBounds = (helper.GetCurrentMonitorWorkArea(mBackendWindowHelper.get(), mWindow));
                ForDim2(dim)
                    bounds.position[dim] =
                        monitorBounds.Center()[dim] - bounds.size[dim] / 2;
            }
            else if (params.appWindowParams.windowGeometry.positionMode ==
                     HelloImGui::WindowPositionMode::FromCoords)
            {
                ForDim2(dim) 
                    bounds.position[dim] = (int)((float)bounds.position[dim] * scaleFactor);
            }
            mBackendWindowHelper->SetWindowBounds(mWindow, bounds);
        }
    }
}

void AbstractRunner::UpdateWindowGeometryOnSecondFrame()
{
    FinishAutoSize_IfRequired();
    MakeWindowSizeRelativeTo96Ppi_IfRequired();
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
    // Part 1: store scale factor, so that font size is multiplied by this factor when loading
    // (this is for windows High DPI screen.     // cf https://github.com/pthom/imgui_bundle/issues/7 )
    //
    // Note: On Mac Retina screen, macOS "hides" the DPI, and we are using virtual pixels)
    //       => see macOS_BackingScaleFactor inside imgui_default_settings.cpp
    {
        float scaleFactor = mBackendWindowHelper->GetWindowDpiScaleFactor(mWindow);
        params.appWindowParams.outWindowDpiFactor = scaleFactor;
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

    // High DPI handling on windows & linux
    // Part 2: resize ImGui style
    if (params.appWindowParams.outWindowDpiFactor > 1.f)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(params.appWindowParams.outWindowDpiFactor);
    }
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

    params.isIdling = false;
    if (params.fpsIdle > 0.f)
    {
        double beforeWait = ClockSeconds();
        double waitTimeout = 1. / (double) params.fpsIdle;
        mBackendWindowHelper->WaitForEventTimeout(waitTimeout);

        double afterWait = ClockSeconds();
        double waitDuration = (afterWait - beforeWait);
        double waitIdleExpected = 1. / params.fpsIdle;
        params.isIdling = (waitDuration > waitIdleExpected * 0.9);
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
