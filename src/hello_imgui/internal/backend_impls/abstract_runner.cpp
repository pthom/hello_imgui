#include "hello_imgui/internal/backend_impls/abstract_runner.h"
#include "hello_imgui/internal/hello_imgui_ini_settings.h"
#include "hello_imgui/internal/docking_details.h"
#include "hello_imgui/internal/menu_statusbar.h"
#include "hello_imgui/image_from_asset.h"
#include "hello_imgui/hello_imgui_theme.h"
#include "hello_imgui/internal/clock_seconds.h"
#include "imgui.h"

#include "hello_imgui/internal/imgui_global_context.h" // must be included before imgui_internal.h
#include "imgui_internal.h"

#include <chrono>
#include <cassert>
#include <fstream>
#include <sstream>
#include <thread>

#ifdef HELLOIMGUI_MACOS
#import <AppKit/NSScreen.h>
#endif

#ifdef HELLOIMGUI_HAS_OPENGL
#include "hello_imgui/hello_imgui_include_opengl.h"
#include "backends/imgui_impl_opengl3.h"
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif



namespace HelloImGui
{
// Encapsulated inside hello_imgui_screenshot.cpp
void setFinalAppWindowScreenshotRgbBuffer(const ImageBuffer& b);


AbstractRunner::AbstractRunner(RunnerParams &params_)
    : params(params_)
{};

// Advanced: ImGui_ImplOpenGL3_CreateFontsTexture might cause an OpenGl error when the font texture is too big
// We detect it soon after calling ImGui::NewFrame by setting mPotentialFontLoadingError.
// In that case, we try to set FontGlobalScale to 1 if it is < 1 (i.e stop increasing the font
// size for a crisper rendering) and try to reload the fonts.
// This only works if the user provided callback LoadAdditionalFonts() uses DpiFontLoadingFactor()
// to multiply its font size.
void AbstractRunner::ReloadFontIfFailed()
{
    fprintf(stderr, "Detected a potential font loading error! You might try to reduce the number of loaded fonts and/or their size!\n");
#ifdef HELLOIMGUI_HAS_OPENGL
    if (ImGui::GetIO().FontGlobalScale < 1.f)
    {
        fprintf(stderr,
                "Trying to solve the font loading error by changing ImGui::GetIO().FontGlobalScale from %f to 1.f! Font rendering might be less crisp...\n",
                ImGui::GetIO().FontGlobalScale
                );
        ImGui::GetIO().FontGlobalScale = 1.f;
        ImGui::GetIO().Fonts->Clear();
        params.callbacks.LoadAdditionalFonts();
        ImGui::GetIO().Fonts->Build();
        ImGui_ImplOpenGL3_CreateFontsTexture();
    }
#endif
}

void AbstractRunner::Run()
{
    Setup();

    mIdxFrame = 0;
#ifdef HELLOIMGUI_MOBILEDEVICE
    while (true)
        CreateFramesAndRender();
#else
    try
    {
        while (!params.appShallExit)
            CreateFramesAndRender();

        TearDown(false);
    }
    catch(std::exception&)
    {
        // Late handling of user exceptions: TearDown backend and rethrow
        TearDown(true);
        throw;
    }
#endif
}

#ifdef HELLO_IMGUI_IMGUI_SHARED
static void*   MyMallocWrapper(size_t size, void* user_data)    { IM_UNUSED(user_data); return malloc(size); }
static void    MyFreeWrapper(void* ptr, void* user_data)        { IM_UNUSED(user_data); free(ptr); }
#endif

void AbstractRunner::PrepareWindowGeometry()
{
    mGeometryHelper = std::make_unique<WindowGeometryHelper>(
        params.appWindowParams.windowGeometry,
        params.appWindowParams.restorePreviousGeometry,
        IniPartsFilename()
        );
    auto windowBounds = mGeometryHelper->AppWindowBoundsInitial(mBackendWindowHelper->GetMonitorsWorkAreas());
    if (params.appWindowParams.restorePreviousGeometry &&
        HelloImGuiIniSettings::LoadLastRunWindowBounds(IniPartsFilename()).has_value())
        params.appWindowParams.windowGeometry.positionMode = WindowPositionMode::FromCoords;
    params.appWindowParams.windowGeometry.position = windowBounds.position;
    params.appWindowParams.windowGeometry.size = windowBounds.size;
}

bool AbstractRunner::WantAutoSize()
{
#ifdef __EMSCRIPTEN__
    return false;
#else
    // AutoSize at startup happens on the second frame, 
    // since the window may have been resized to handle DPI scaling on the first frame
    bool autosizeAtStartup = (mIdxFrame == 1) &&  !mGeometryHelper->HasInitialWindowSizeInfo();
    bool autosizeAtNextFrame = params.appWindowParams.windowGeometry.resizeAppWindowAtNextFrame;
    return autosizeAtStartup || autosizeAtNextFrame;
#endif
}


bool AbstractRunner::ShallSizeWindowRelativeTo96Ppi() 
{
    bool shallSizeRelativeTo96Ppi;
    {
        bool doRestorePreviousGeometry = (params.appWindowParams.restorePreviousGeometry &&
            HelloImGuiIniSettings::LoadLastRunWindowBounds(IniPartsFilename()).has_value()
                                          );

        bool isWindowPpiRelativeSize = (params.appWindowParams.windowGeometry.windowSizeMeasureMode ==
                                        HelloImGui::WindowSizeMeasureMode::RelativeTo96Ppi);

        bool isStandardSizeMode = (
            params.appWindowParams.windowGeometry.windowSizeState == HelloImGui::WindowSizeState::Standard
            || params.appWindowParams.windowGeometry.windowSizeState == HelloImGui::WindowSizeState::Minimized);

        shallSizeRelativeTo96Ppi =
            isStandardSizeMode && isWindowPpiRelativeSize && !doRestorePreviousGeometry;
    }
    return shallSizeRelativeTo96Ppi;
}

float AbstractRunner::ImGuiDefaultFontGlobalScale()
{
    float fontSizeIncreaseFactor = 1.f;

#ifdef __EMSCRIPTEN__
    // Query the brower to ask for devicePixelRatio
    double windowDevicePixelRatio = EM_ASM_DOUBLE( {
        scale = window.devicePixelRatio;
        return scale;
    }
    );
    printf("window.devicePixelRatio=%lf\n", windowDevicePixelRatio);

    fontSizeIncreaseFactor = windowDevicePixelRatio;
#endif
#ifdef HELLOIMGUI_MACOS
    // Crisp fonts on MacOS:
    // cf https://github.com/ocornut/imgui/issues/5301
    // Issue with macOS is that it pretends screen has 2x fewer pixels than it actually does.
    // This simplifies application development in most cases, but in our case we happen to render fonts at 1x scale
    // while screen renders at 2x scale.
    fontSizeIncreaseFactor = (float) NSScreen.mainScreen.backingScaleFactor;
#endif

    float defaultFontGlobalScale = 1.0f / fontSizeIncreaseFactor;
    return defaultFontGlobalScale;
}

float AbstractRunner::DpiWindowSizeFactor()
{
    return mBackendWindowHelper->GetWindowSizeDpiScaleFactor(mWindow);
}


// If we want a font to visually render like a font size of 14,
// we need to multiply its size by this factor
float AbstractRunner::DpiFontLoadingFactor()
{
    float k = DpiWindowSizeFactor() * 1.f / ImGui::GetIO().FontGlobalScale;
    return k;
}

void AbstractRunner::MakeWindowSizeRelativeTo96Ppi_IfRequired()
{
    if (ShallSizeWindowRelativeTo96Ppi())
    {
        float scaleFactor = DpiWindowSizeFactor();
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
                auto monitorBounds = (mGeometryHelper->GetCurrentMonitorWorkArea(mBackendWindowHelper.get(), mWindow));
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

// This will change the window size if we want a size relative to 96ppi and rescale the imgui style
void AbstractRunner::HandleDpiOnSecondFrame()
{
    MakeWindowSizeRelativeTo96Ppi_IfRequired();
    // High DPI handling on windows & linux
    {
        float dpiScale = DpiWindowSizeFactor();
        if ( dpiScale > 1.f)
        {
            ImGuiStyle& style = ImGui::GetStyle();
            style.ScaleAllSizes(dpiScale);
        }
    }
}


std::string AbstractRunner::IniPartsFilename()
{
    auto _stringToSaneFilename=[](const std::string& s, const std::string& extension) -> std::string
    {
        std::string filenameSanitized;
        for (char c : s)
            if (isalnum(c))
                filenameSanitized += c;
            else
                filenameSanitized += "_";
        filenameSanitized += extension;
        return filenameSanitized;
    };

    if (! params.iniFilename.empty())
        return params.iniFilename.c_str();
    else
    {
        if (params.iniFilename_useAppWindowTitle && !params.appWindowParams.windowTitle.empty())
        {
            std::string iniFilenameSanitized = _stringToSaneFilename(params.appWindowParams.windowTitle, ".ini");
            return iniFilenameSanitized;
        }
        else
            return "imgui.ini";
    }
}

void AbstractRunner::LayoutSettings_SwitchLayout(const std::string& layoutName)
{
    if (params.dockingParams.layoutName == layoutName)
        return;

    // if we previously loaded another layout, save its settings before changing
    {
        static bool isFirstLayoutSwitch = true;
        if (! isFirstLayoutSwitch)
            LayoutSettings_Save();
        isFirstLayoutSwitch = false;
    }

    if (layoutName.empty())
        return;

    const DockingParams* wantedLayout = nullptr;
    for (const auto& layout: params.alternativeDockingLayouts)
        if (layout.layoutName == layoutName)
            wantedLayout = &layout;

    if (wantedLayout == nullptr)
    {
        fprintf(stderr, "Can't switch to non existing layout %s\n", layoutName.c_str());
        return;
    }

    std::vector<DockingParams> newAlternativeDockingLayouts;
    newAlternativeDockingLayouts.push_back(params.dockingParams);
    params.dockingParams = *wantedLayout;
    for (const auto& layout: params.alternativeDockingLayouts)
        if (layout.layoutName != layoutName)
            newAlternativeDockingLayouts.push_back(layout);
    params.alternativeDockingLayouts = newAlternativeDockingLayouts;
}

// Those Layout_XXX functions are called before ImGui::NewFrame()
void AbstractRunner::LayoutSettings_HandleChanges()
{
    static std::string lastLoadedLayout = "";
    if (params.dockingParams.layoutName != lastLoadedLayout)
    {
        LayoutSettings_Load();
        lastLoadedLayout = params.dockingParams.layoutName;
    }
}
void AbstractRunner::LayoutSettings_Load()
{
    HelloImGuiIniSettings::LoadImGuiSettings(IniPartsFilename(), params.dockingParams.layoutName);
    HelloImGuiIniSettings::LoadDockableWindowsVisibility(IniPartsFilename(), &params.dockingParams);

    // SetLayoutResetIfNeeded() may set params.dockingParams.layoutReset = true
    // (which will cause the layout to be recreated from scratch, overriding user changes)
    SetLayoutResetIfNeeded();
}
void AbstractRunner::LayoutSettings_Save()
{
    HelloImGuiIniSettings::SaveImGuiSettings(IniPartsFilename(), params.dockingParams.layoutName);
    HelloImGuiIniSettings::SaveDockableWindowsVisibility(IniPartsFilename(), params.dockingParams);
}

void AbstractRunner::Setup()
{
    Impl_InitBackend();
    Impl_Select_Gl_Version();

    PrepareWindowGeometry();
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

    ImGui::GetIO().IniFilename = NULL;

    Impl_SetupImgGuiContext();
    params.callbacks.SetupImGuiConfig();
    if (params.imGuiWindowParams.enableViewports)
    {
#ifndef __EMSCRIPTEN__
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif
    }
    params.callbacks.SetupImGuiStyle();

    // This should be done before Impl_SetupPlatformRendererBindings()
    // because, in the case of glfw ImGui_ImplGlfw_InstallCallbacks
    // will chain the user callbacks with ImGui callbacks; and PostInit()
    // is a good place for the user to install callbacks
    if (params.callbacks.PostInit)
        params.callbacks.PostInit();

    Impl_SetupPlatformRendererBindings();

    //
    // load fonts & set ImGui::GetIO().FontGlobalScale
    //
    ImGui::GetIO().Fonts->Clear();
    // (On macOS with HighDPI) FontGlobalScale may be set to 0.5
    ImGui::GetIO().FontGlobalScale = this->ImGuiDefaultFontGlobalScale();
    // LoadAdditionalFonts will load fonts and resize them by 1./FontGlobalScale
    // (if and only if it uses HelloImGui::LoadFontTTF instead of ImGui's font loading functions)
    params.callbacks.LoadAdditionalFonts();
    ImGui::GetIO().Fonts->Build();
    {
        // Reset FontGlobalScale if we did not use HelloImGui font loading mechanism
        if (! HelloImGui::ImGuiDefaultSettings::DidCallHelloImGuiLoadFontTTF())
        {
            float dpiFactor = mBackendWindowHelper->GetWindowSizeDpiScaleFactor(mWindow);
            ImGui::GetIO().FontGlobalScale = dpiFactor;
        }
    }

    DockingDetails::ConfigureImGuiDocking(params.imGuiWindowParams);
    HelloImGuiIniSettings::LoadHelloImGuiMiscSettings(IniPartsFilename(), &params);
    SetLayoutResetIfNeeded();

    ImGuiTheme::ApplyTweakedTheme(params.imGuiWindowParams.tweakedTheme);
}

void AbstractRunner::SetLayoutResetIfNeeded()
{
    if (params.imGuiWindowParams.defaultImGuiWindowType == DefaultImGuiWindowType::ProvideFullScreenDockSpace)
    {
        if (params.dockingParams.layoutCondition == DockingLayoutCondition::FirstUseEver)
        {
            if (!HelloImGuiIniSettings::HasUserDockingSettingsInImguiSettings(IniPartsFilename(), params.dockingParams))
                params.dockingParams.layoutReset = true;
            else
                params.dockingParams.layoutReset = false;
        }
        else if (params.dockingParams.layoutCondition == DockingLayoutCondition::ApplicationStart)
            params.dockingParams.layoutReset = true;
        else if (params.dockingParams.layoutCondition == DockingLayoutCondition::Never)
            params.dockingParams.layoutReset = false;
    }
}

void AbstractRunner::RenderGui()
{
    DockingDetails::ProvideWindowOrDock(params);

    if (params.imGuiWindowParams.showMenuBar)
        Menu_StatusBar::ShowMenu(params);

    if (params.callbacks.ShowGui)
    {
        bool wantAutoSize = WantAutoSize();

        if (wantAutoSize)
            ImGui::BeginGroup();

        params.callbacks.ShowGui();

        if (wantAutoSize)
        {
            ImGui::EndGroup();
            ImVec2 userWidgetsSize = ImGui::GetItemRectSize();
            mGeometryHelper->TrySetWindowSize(mBackendWindowHelper.get(), mWindow, userWidgetsSize);
            mWasWindowAutoResizedOnPreviousFrame = true;
        }
    }

    DockingDetails::ShowDockableWindows(params.dockingParams.dockableWindows);

    if (params.imGuiWindowParams.showStatusBar)
        Menu_StatusBar::ShowStatusBar(params);

    Theme_WindowGui(params.imGuiWindowParams.tweakedTheme);

    DockingDetails::CloseWindowOrDock(params.imGuiWindowParams);
}


void AbstractRunner::CreateFramesAndRender()
{
    LayoutSettings_HandleChanges();

    // Note about the application window initial placement and sizing 
    // i/   On the first frame (mIdxFrame==0), we create a window, and use the user provided size (if provided). The window is initially hidden.
    //      (this was done much sooner by mBackendWindowHelper)
    // ii/  On the second frame (mIdxFrame == 1), we may multiply this size by the Dpi factor (if > 1), to handle windows and linux High DPI
    // iii/ At the end of the second frame, we measure the size of the widgets and use it as the application window size, if the user required auto size
    // iv/  At the beginning of the third frame (mIdxFrame==2 / mWasWindowAutoResizedOnPreviousFrame), we may apply the auto-size and recenter the window to the center of the monitor
    // v/   At the 4th frame (mIdxFrame >= 3), we finally show the window
    // Phew...

    // ii/ On the second frame (mIdxFrame == 1), we may multiply this size by the Dpi factor (if > 1), to handle windows and linux High DPI
    if (mIdxFrame == 1)
    {
        // We might resize the window on the second frame on window and linux
        // (and rescale ImGui style)
        HandleDpiOnSecondFrame();
    }

    // iv/ At the beginning of the third frame (mIdxFrame==2 / mWasWindowAutoResizedOnPreviousFrame), we may apply the auto-size and recenter the window to the center of the monitor
    if (mWasWindowAutoResizedOnPreviousFrame)
    {
        // The window was resized on last frame
        // We should now recenter the window if needed and ensure it fits on the monitor
        mGeometryHelper->EnsureWindowFitsMonitor(mBackendWindowHelper.get(), mWindow);

        // if this is the third frame, and the user wanted a centered window, let's recenter it
        // we do this on the third frame (mIdxFrame == 2), since the initial autosize happens on the second
        // (see WantAutoSize())
        if (params.appWindowParams.windowGeometry.positionMode == HelloImGui::WindowPositionMode::MonitorCenter && (mIdxFrame == 2))
            mGeometryHelper->CenterWindowOnMonitor(mBackendWindowHelper.get(), mWindow);

        mWasWindowAutoResizedOnPreviousFrame = false;
        params.appWindowParams.windowGeometry.resizeAppWindowAtNextFrame = false;
    }


    static bool lastHiddenState = false;

    // v/   At the 4th frame (mIdxFrame >= 3), we finally show the window
    if (mIdxFrame == 3)
    {
        if (params.appWindowParams.hidden)
            mBackendWindowHelper->HideWindow(mWindow);
        else
            mBackendWindowHelper->ShowWindow(mWindow);
        lastHiddenState = params.appWindowParams.hidden;
    }
    // On subsequent frames, we take into account user modifications of appWindowParams.hidden
    if (mIdxFrame > 3)
    {
        if (params.appWindowParams.hidden != lastHiddenState)
        {
            lastHiddenState = params.appWindowParams.hidden;
            if (params.appWindowParams.hidden)
                mBackendWindowHelper->HideWindow(mWindow);
            else
                mBackendWindowHelper->ShowWindow(mWindow);
        }
    }

#ifndef __EMSCRIPTEN__
    // Idling for non emscripten, where HelloImGui is responsible for the main loop.
    // This form of idling will call WaitForEventTimeout(), which may call sleep():
    IdleBySleeping();
#endif

    // Poll Events (this fills GImGui.InputEventsQueue)
    Impl_PollEvents();

#ifdef __EMSCRIPTEN__
    // Idling for emscripten: early exit if idling
    // This test should be done after calling Impl_PollEvents() since it checks the event queue for incoming events!
    if (ShallIdleThisFrame_Emscripten())
        return;
#endif

    //
    // Rendering logic
    //
    Impl_NewFrame_3D();
    Impl_NewFrame_Backend();
    {
        // Workaround against SDL clock that sometimes leads to io.DeltaTime=0.f on emscripten
        // (which fails to an `IM_ASSERT(io.DeltaTime) > 0` in ImGui::NewFrame())
        //
        // This can be removed once the commit 07490618 was merged into imgui docking branch
        // (see https://github.com/ocornut/imgui/commit/07490618ae47fdb9f625565fbb183593170a6a72)
        auto & io = ImGui::GetIO();
        if (io.DeltaTime <= 0.f)
            io.DeltaTime = 1.f / 60.f;
    }

    if (params.callbacks.PreNewFrame)
        params.callbacks.PreNewFrame();

    ImGui::NewFrame();

    // Fight against potential font loading issue
    // Fonts are transferred to the GPU one the first call to ImGui::NewFrame()
    // We might detect an OpenGL error if the font texture was too big for the GPU
    bool foundPotentialFontLoadingError = false;
    {
#ifdef HELLOIMGUI_HAS_OPENGL
        if (mIdxFrame == 0)
        {
            auto error = glGetError();
            if (error != 0)
                foundPotentialFontLoadingError = true;
        }
#endif
    }

    // iii/ At the end of the second frame, we measure the size of the widgets and use it as the application window size, if the user required auto size
    // ==> Note: RenderGui() may measure the size of the window and resize it if mIdxFrame==1
    RenderGui();

    ImGui::Render();
    Impl_Frame_3D_ClearColor();
    Impl_RenderDrawData_To_3D();

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        Impl_UpdateAndRenderAdditionalPlatformWindows();

    Impl_SwapBuffers();

    if (foundPotentialFontLoadingError)
        ReloadFontIfFailed();

    mIdxFrame += 1;
}

// Idling for non emscripten, where HelloImGui is responsible for the main loop.
// This form of idling will call WaitForEventTimeout(), which may call sleep():
void AbstractRunner::IdleBySleeping()
{
    assert(params.fpsIdling.fpsIdle >= 0.f);
    params.fpsIdling.isIdling = false;
    if ((params.fpsIdling.fpsIdle > 0.f) && params.fpsIdling.enableIdling)
    {
        double beforeWait = Internal::ClockSeconds();
        double waitTimeout = 1. / (double) params.fpsIdling.fpsIdle;
        mBackendWindowHelper->WaitForEventTimeout(waitTimeout);

        double afterWait = Internal::ClockSeconds();
        double waitDuration = (afterWait - beforeWait);
        double waitIdleExpected = 1. / params.fpsIdling.fpsIdle;
        params.fpsIdling.isIdling = (waitDuration > waitIdleExpected * 0.9);
    }
}

// Logic for idling under emscripten
// (this is adapted for emscripten, since CreateFramesAndRender is called 60 times per second by the browser,
//  and a sleep would actually could a javascript busy loop!)
// This test should be done after calling Impl_PollEvents() since it checks the event queue for incoming events!
bool AbstractRunner::ShallIdleThisFrame_Emscripten()
{
    ImGuiContext& g = *GImGui;
    bool hasInputEvent =  ! g.InputEventsQueue.empty();

    if (! params.fpsIdling.enableIdling)
    {
        params.fpsIdling.isIdling = false;
        return false;
    }

    static double lastRefreshTime = 0.;
    double now = Internal::ClockSeconds();

    bool shallIdleThisFrame = false;
    if (hasInputEvent)
    {
        params.fpsIdling.isIdling = false;
        shallIdleThisFrame = false;
    }
    else
    {
        params.fpsIdling.isIdling = true;
        if ((now - lastRefreshTime) < 1. / params.fpsIdling.fpsIdle)
            shallIdleThisFrame = true;
        else
            shallIdleThisFrame = false;
    }

    if (! shallIdleThisFrame)
        lastRefreshTime = now;

    return shallIdleThisFrame;
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

void AbstractRunner::TearDown(bool gotException)
{
    if (! gotException)
    {
        // Store screenshot before exiting
        {
            ImageBuffer b = ScreenshotRgb();
            setFinalAppWindowScreenshotRgbBuffer(b);
        }
        if (params.appWindowParams.restorePreviousGeometry)
            HelloImGuiIniSettings::SaveLastRunWindowBounds(IniPartsFilename(),
                                                           mBackendWindowHelper->GetWindowBounds(mWindow));
        LayoutSettings_Save();
        HelloImGuiIniSettings::SaveHelloImGuiMiscSettings(IniPartsFilename(), params);
    }

    HelloImGui::internal::Free_ImageFromAssetMap();
    if (params.callbacks.BeforeExit)
        params.callbacks.BeforeExit();
    Impl_Cleanup();
}


void AbstractRunner::SaveUserPref(const std::string& userPrefName, const std::string& userPrefContent)
{
    HelloImGuiIniSettings::SaveUserPref(IniPartsFilename(), userPrefName, userPrefContent);
}
std::string AbstractRunner::LoadUserPref(const std::string& userPrefName)
{
    return HelloImGuiIniSettings::LoadUserPref(IniPartsFilename(), userPrefName);
}

}  // namespace HelloImGui
