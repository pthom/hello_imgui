#include "hello_imgui/internal/backend_impls/abstract_runner.h"
#include "hello_imgui/hello_imgui_theme.h"
#include "hello_imgui/internal/borderless_movable.h"
#include "hello_imgui/internal/clock_seconds.h"
#include "hello_imgui/internal/docking_details.h"
#include "hello_imgui/internal/hello_imgui_ini_settings.h"
#include "hello_imgui/internal/hello_imgui_ini_any_parent_folder.h"
#include "hello_imgui/internal/menu_statusbar.h"
#include "hello_imgui/internal/platform/ini_folder_locations.h"
#include "hello_imgui/internal/inicpp.h"
#include "hello_imgui/internal/poor_man_log.h"
#include "imgui.h"

#include "hello_imgui/internal/imgui_global_context.h" // must be included before imgui_internal.h
#include "imgui_internal.h"

#include "hello_imgui_test_engine_integration/test_engine_integration.h"

#ifdef IMGUI_TEST_ENGINE_WITH_PYTHON_GIL
#include "imgui_test_engine/imgui_te_python_gil.h"
#define SCOPED_RELEASE_GIL_ON_MAIN_THREAD ImGuiTestEnginePythonGIL::ReleaseGilOnMainThread_Scoped _gilRelease
#else
#define SCOPED_RELEASE_GIL_ON_MAIN_THREAD
#endif

#include <chrono>
#include <cassert>
#include <filesystem>
#include <cstdio>
#include <optional>

#if __APPLE__
#include <TargetConditionals.h>
#endif

#ifdef HELLOIMGUI_MACOS
#import <AppKit/NSScreen.h>
#endif
#if TARGET_OS_IOS
#include "hello_imgui/internal/platform/getAppleBundleResourcePath.h"
#endif

#ifdef HELLOIMGUI_HAS_OPENGL
#include "hello_imgui/hello_imgui_include_opengl.h"
#include "imgui_impl_opengl3.h"
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "rendering_opengl3.h"
#include "rendering_metal.h"
#include "rendering_vulkan.h"
#include "rendering_dx11.h"
#include "rendering_dx12.h"
#include "rendering_null.h"

//
// NOTE: AbstractRunner should *not* care in any case of:
//   - the platform backend (SDL, Glfw, ...)
//   - the rendering backend (OpenGL, Metal, ...)
// For legacy reasons, there are still few references to OpenGL in this file, but this needs to be refactored out.
//


//#define ENABLE_DPI_LOG  // Enable or disable logging for DPI info
//#define ENABLE_DPI_LOG

#ifdef ENABLE_DPI_LOG
#define DpiLog PoorManLog
#else
#define DpiLog(...)
#endif


namespace HelloImGui
{
// Encapsulated inside hello_imgui_screenshot.cpp
void setFinalAppWindowScreenshotRgbBuffer(const ImageBuffer& b);

// Encapsulated inside hello_imgui_font.cpp
bool _reloadAllDpiResponsiveFonts();
bool ShouldRemoteDisplay();

// Encapsulated inside docking_details.cpp
void ShowThemeTweakGuiWindow_Static();


struct AbstractRunnerStatics
{
    std::string lastLoadedLayout;
    bool isFirstLayoutSwitch = true;
    bool lastHiddenState = false;
    double timeLastEvent = -1.;
    double lastRefreshTime = 0.;
};

static AbstractRunnerStatics gStatics;

static void ResetAbstractRunnerStatics() { gStatics = AbstractRunnerStatics(); }



AbstractRunner::AbstractRunner(RunnerParams &params_)
: params(params_) {}


AbstractRunner::~AbstractRunner()
{
    ResetAbstractRunnerStatics();
}


#ifndef USEHACK
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
#endif

#ifdef HELLO_IMGUI_IMGUI_SHARED
static void*   MyMallocWrapper(size_t size, void* user_data)    { IM_UNUSED(user_data); return malloc(size); }
static void    MyFreeWrapper(void* ptr, void* user_data)        { IM_UNUSED(user_data); free(ptr); }
#endif

void AbstractRunner::PrepareWindowGeometry()
{
    mGeometryHelper = std::make_unique<WindowGeometryHelper>(
        params.appWindowParams.windowGeometry,
        params.appWindowParams.restorePreviousGeometry,
        IniSettingsLocation(params)
        );
    auto windowBounds = mGeometryHelper->AppWindowBoundsInitial(mBackendWindowHelper->GetMonitorsWorkAreas());
    if (params.appWindowParams.restorePreviousGeometry &&
        HelloImGuiIniSettings::LoadLastRunWindowBounds(IniSettingsLocation(params)).has_value())
        params.appWindowParams.windowGeometry.positionMode = WindowPositionMode::FromCoords;
    params.appWindowParams.windowGeometry.position = windowBounds.position;
    params.appWindowParams.windowGeometry.size = windowBounds.size;
}


// If needed, change the size to match the current DPI versus the DPI when the size was saved
// (we want the window to "look" as big as it was when saved, even if the DPI has changed,
//  or if we are on a different monitor / computer / OS)
void AbstractRunner::AdjustWindowBoundsAfterCreation_IfDpiChangedBetweenRuns()
{
    bool reloadLastSize = params.appWindowParams.restorePreviousGeometry &&
                          HelloImGuiIniSettings::LoadLastRunWindowBounds(IniSettingsLocation(params)).has_value();
    if (!reloadLastSize)
        return;

    std::optional<float> lastRunDpiWindowSizeFactorOpt = HelloImGuiIniSettings::LoadLastRunDpiWindowSizeFactor(IniSettingsLocation(params));
    if (!lastRunDpiWindowSizeFactorOpt.has_value())
        lastRunDpiWindowSizeFactorOpt = 1.f;

    float lastRunDpiWindowSizeFactor = lastRunDpiWindowSizeFactorOpt.value();
    float currentDpiWindowSizeFactor = params.dpiAwareParams.dpiWindowSizeFactor;
    float ratio = currentDpiWindowSizeFactor / lastRunDpiWindowSizeFactor;
    bool isSane = ratio > 0.25f && ratio < 4.f;
    if (isSane && ratio != 1.f)
    {
        auto bounds = mBackendWindowHelper->GetWindowBounds(mWindow);
        bounds.size = {(int)((float)bounds.size[0] * ratio),
                       (int)((float)bounds.size[1] * ratio)};
        bounds.position = {
            (int)((float)bounds.position[0] * ratio),
            (int)((float)bounds.position[1] * ratio)
        };
        mBackendWindowHelper->SetWindowBounds(mWindow, bounds);
    }
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

void AbstractRunner::ChangeWindowSize(HelloImGui::ScreenSize windowSize)
{
    auto bounds = mBackendWindowHelper->GetWindowBounds(mWindow);
    bounds.size = windowSize;
    this->setWasWindowResizedByCodeDuringThisFrame();
    mBackendWindowHelper->SetWindowBounds(mWindow, bounds);
}

bool AbstractRunner::ShallSizeWindowRelativeTo96Ppi() 
{
    bool shallSizeRelativeTo96Ppi;
    {
        bool doRestorePreviousGeometry = (params.appWindowParams.restorePreviousGeometry &&
            HelloImGuiIniSettings::LoadLastRunWindowBounds(IniSettingsLocation(params)).has_value()
                                          );

        bool isWindowPpiRelativeSize = (params.appWindowParams.windowGeometry.windowSizeMeasureMode ==
                                        HelloImGui::WindowSizeMeasureMode::RelativeTo96Ppi);

        bool isStandardSizeMode = (
            params.appWindowParams.windowGeometry.windowSizeState == HelloImGui::WindowSizeState::Standard
            || params.appWindowParams.windowGeometry.windowSizeState == HelloImGui::WindowSizeState::Minimized);

        bool isFullScreen =
            params.appWindowParams.windowGeometry.fullScreenMode != HelloImGui::FullScreenMode::NoFullScreen;

        shallSizeRelativeTo96Ppi =
            isStandardSizeMode && isWindowPpiRelativeSize && !doRestorePreviousGeometry && !isFullScreen;

    }
    return shallSizeRelativeTo96Ppi;
}

void ReadDpiAwareParams(DpiAwareParams* dpiAwareParams)
{
    // cf dpi_aware.h
    //
    // Hello ImGui will try its best to automatically handle DPI scaling for you.
    // It does this by setting two values:
    //
    // - `dpiWindowSizeFactor`:
    //        factor by which window size should be multiplied
    //
    // - `fontRenderingScale`:
    //     factor by which fonts glyphs should be scaled at rendering time
    //     (typically 1 on windows, and 0.5 on macOS retina screens)
    //
    //    By default, Hello ImGui will compute them automatically,
    //    when dpiWindowSizeFactor and fontRenderingScale are set to 0.
    //
    // How to set those values manually:
    // ---------------------------------
    // If it fails (i.e. your window and/or fonts are too big or too small),
    // you may set them manually:
    //    (1) Either by setting them programmatically in your application
    //        (set their values in `runnerParams.dpiAwareParams`)
    //    (2) Either by setting them in a `hello_imgui.ini` file in the current folder, or any of its parent folders.
    //       (this is useful when you want to set them for a specific app or set of apps, without modifying the app code)
    // Note: if several methods are used, the order of priority is (1) > (2)
    //
    // Example content of a ini file:
    // ------------------------------
    //     [DpiAwareParams]
    //     dpiWindowSizeFactor=2
    //     fontRenderingScale=0.5
    //

    if (dpiAwareParams->dpiWindowSizeFactor == 0.f)
    {
        auto dpiWindowSizeFactor = HelloImGuiIniAnyParentFolder::readFloatValue("DpiAwareParams", "dpiWindowSizeFactor");
        if (dpiWindowSizeFactor.has_value())
            dpiAwareParams->dpiWindowSizeFactor = dpiWindowSizeFactor.value();
    }

    if (dpiAwareParams->fontRenderingScale == 0.f)
    {
        auto fontRenderingScale = HelloImGuiIniAnyParentFolder::readFloatValue("DpiAwareParams", "fontRenderingScale");
        if (fontRenderingScale.has_value())
            dpiAwareParams->fontRenderingScale = fontRenderingScale.value();
    }

    if (dpiAwareParams->fontOversampleH == 0)
    {
        auto fontOversampleH = HelloImGuiIniAnyParentFolder::readIntValue("DpiAwareParams", "fontOversampleH");
        if (fontOversampleH.has_value())
            dpiAwareParams->fontOversampleH = fontOversampleH.value();
    }
    if (dpiAwareParams->fontOversampleV == 0)
    {
        auto fontOversampleV = HelloImGuiIniAnyParentFolder::readIntValue("DpiAwareParams", "fontOversampleV");
        if (fontOversampleV.has_value())
            dpiAwareParams->fontOversampleV = fontOversampleV.value();
    }
}


void _LogDpiParams(const std::string& origin, const HelloImGui::DpiAwareParams& dpiAwareParams)
{
	auto &io = ImGui::GetIO();
	std::stringstream msg;
	DpiLog("DpiAwareParams: %s\n", origin.c_str());
	DpiLog("    dpiWindowSizeFactor=%f\n", dpiAwareParams.dpiWindowSizeFactor);
	DpiLog("    fontRenderingScale=%f\n", dpiAwareParams.fontRenderingScale);
	DpiLog("    DpiFontLoadingFactor()=%f\n", dpiAwareParams.DpiFontLoadingFactor());
	DpiLog("        (ImGui FontGlobalScale: %f)\n", io.FontGlobalScale);
	DpiLog("	    (ImGui DisplayFramebufferScale=%f, %f)\n", io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
}


bool AbstractRunner::CheckDpiAwareParamsChanges()
{
    auto& dpiAwareParams = params.dpiAwareParams;
    auto& io = ImGui::GetIO();

	// Check changes
	bool didFontGlobalScaleChange = dpiAwareParams.fontRenderingScale != io.FontGlobalScale;
	if (didFontGlobalScaleChange)
	{
		DpiLog("Warning: didFontGlobalScaleChange=:true\n");
		dpiAwareParams.fontRenderingScale = io.FontGlobalScale;
	}

    bool didDpiAwareParamsChangeOnRemoteServer = mRemoteDisplayHandler.CheckDpiAwareParamsChanges();

	if (didFontGlobalScaleChange || didDpiAwareParamsChangeOnRemoteServer)
	{
		DpiLog("New DpiAwareParams:\n");
		_LogDpiParams("_CheckDpiAwareParamsChanges (changed!)", dpiAwareParams);
		return true;
	}
	else
		return false;
}


float _DefaultOsFontRenderingScale()
{
    float fontSizeIncreaseFactor = 1.f;

    #ifdef __EMSCRIPTEN__
        // Query the brower to ask for devicePixelRatio
        double windowDevicePixelRatio = EM_ASM_DOUBLE( {
            var scale = window.devicePixelRatio;
            return scale;
        }
        );
        printf("window.devicePixelRatio=%lf\n", windowDevicePixelRatio);

        fontSizeIncreaseFactor = windowDevicePixelRatio;
    #endif
    #ifdef HELLOIMGUI_MACOS
        // Crisp fonts on macOS:
        // cf https://github.com/ocornut/imgui/issues/5301
        // Issue with macOS is that it pretends screen has 2x fewer pixels than it actually does.
        // This simplifies application development in most cases, but in our case we happen to render fonts at 1x scale
        // while screen renders at 2x scale.
        fontSizeIncreaseFactor = (float) NSScreen.mainScreen.backingScaleFactor;
    #endif

    #ifdef HELLOIMGUI_WITH_REMOTE_DISPLAY
    if (HelloImGui::GetRunnerParams()->remoteParams.enableRemoting)
        fontSizeIncreaseFactor = 1.f;
    #endif

    return 1.0f / fontSizeIncreaseFactor;
}


void AbstractRunner::SetupDpiAwareParams()
{
    ReadDpiAwareParams(&params.dpiAwareParams);
    if (params.dpiAwareParams.dpiWindowSizeFactor == 0.f)
    {
        #ifdef HELLOIMGUI_HAS_DIRECTX11
        if (params.rendererBackendType == HelloImGui::RendererBackendType::DirectX11)
        {
            // The current implementation of Dx11 backend does  not support changing the window size
            params.dpiAwareParams.dpiWindowSizeFactor = 1.f;
        }
        #endif
        params.dpiAwareParams.dpiWindowSizeFactor = mBackendWindowHelper->GetWindowSizeDpiScaleFactor(mWindow);
    }

    if (params.dpiAwareParams.fontRenderingScale == 0.f)
    {
        params.dpiAwareParams.fontRenderingScale = _DefaultOsFontRenderingScale();
    }
    ImGui::GetIO().FontGlobalScale = params.dpiAwareParams.fontRenderingScale;

	_LogDpiParams("SetupDpiAwareParams", params.dpiAwareParams);
}


void AbstractRunner::MakeWindowSizeRelativeTo96Ppi_IfRequired()
{
    if (ShallSizeWindowRelativeTo96Ppi())
    {
        float scaleFactor = params.dpiAwareParams.dpiWindowSizeFactor;
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
            setWasWindowResizedByCodeDuringThisFrame();
            mBackendWindowHelper->SetWindowBounds(mWindow, bounds);
        }
    }
}

// This will change the window size if we want a size relative to 96ppi and rescale the imgui style
void AbstractRunner::HandleDpiOnSecondFrame()
{
#ifndef __ANDROID__
    MakeWindowSizeRelativeTo96Ppi_IfRequired();
#endif
    
    // High DPI handling on windows & linux
    if (!ShouldRemoteDisplay())
    {
        float dpiScale =  params.dpiAwareParams.dpiWindowSizeFactor;
        if ( dpiScale > 1.f)
        {
            ImGuiStyle& style = ImGui::GetStyle();
            style.ScaleAllSizes(dpiScale);
        }
    }
}



void AbstractRunner::LayoutSettings_SwitchLayout(const std::string& layoutName)
{
    if (params.dockingParams.layoutName == layoutName)
        return;

    // if we previously loaded another layout, save its settings before changing
    {
        if (! gStatics.isFirstLayoutSwitch)
            LayoutSettings_Save();
        gStatics.isFirstLayoutSwitch = false;
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
    if (params.dockingParams.layoutName != gStatics.lastLoadedLayout)
    {
        LayoutSettings_Load();
        gStatics.lastLoadedLayout = params.dockingParams.layoutName;
    }
}
void AbstractRunner::LayoutSettings_Load()
{
    HelloImGuiIniSettings::LoadImGuiSettings(IniSettingsLocation(params), params.dockingParams.layoutName);
    HelloImGuiIniSettings::LoadDockableWindowsVisibility(IniSettingsLocation(params), &params.dockingParams);

    // SetLayoutResetIfNeeded() may set params.dockingParams.layoutReset = true
    // (which will cause the layout to be recreated from scratch, overriding user changes)
    SetLayoutResetIfNeeded();
}
void AbstractRunner::LayoutSettings_Save()
{
    HelloImGuiIniSettings::SaveImGuiSettings(IniSettingsLocation(params), params.dockingParams.layoutName);
    HelloImGuiIniSettings::SaveDockableWindowsVisibility(IniSettingsLocation(params), params.dockingParams);
}

void AbstractRunner::InitImGuiContext()
{
    IMGUI_CHECKVERSION();
#ifdef HELLO_IMGUI_IMGUI_SHARED
    auto ctx = ImGui::CreateContext();
    GImGui = ctx;
    ImGui::SetCurrentContext(ctx);
    ImGui::SetAllocatorFunctions(MyMallocWrapper, MyFreeWrapper);
#else
    ImGui::CreateContext();
#endif
}

void AbstractRunner::SetImGuiPrefs()
{
    if (params.imGuiWindowParams.enableViewports)
    {
#ifndef __EMSCRIPTEN__
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif
    }

    #ifndef IMGUI_BUNDLE_PYTHON_API
        ImGui::GetIO().IniFilename = NULL;
    #else
        ImGui::GetIO().IniFilename = "";
    #endif
}


extern std::string gMissingBackendErrorMessage; // from hello_imgui.cpp


void AbstractRunner::InitRenderBackendCallbacks()
{
    // We need to take into account the combination of platform and rendering backends
    // In this function, we first check for the rendering backend, and then for the platform backend
    // This function is simple, but long because we have to account for all the combinations + compiler options

    if (params.rendererBackendType == RendererBackendType::OpenGL3)
    {
        #ifdef HELLOIMGUI_HAS_OPENGL
            mRenderingBackendCallbacks = CreateBackendCallbacks_OpenGl3();
        #else
            IM_ASSERT(false && "OpenGL3 backend is not available!");
        #endif
    }
    else if (params.rendererBackendType == RendererBackendType::Metal)
    {
        #ifdef HELLOIMGUI_HAS_METAL
            // Choose between CreateBackendCallbacks_GlfwMetal & CreateBackendCallbacks_SdlMetal
            if (params.platformBackendType == PlatformBackendType::Glfw)
            {
                #ifdef HELLOIMGUI_USE_GLFW3
                    mRenderingBackendCallbacks = CreateBackendCallbacks_GlfwMetal();
                #else
                    IM_ASSERT(false && "Glfw3 backend is not available!");
                #endif
            }
            else if (params.platformBackendType == PlatformBackendType::Sdl)
            {
                #ifdef HELLOIMGUI_USE_SDL2
                    mRenderingBackendCallbacks = CreateBackendCallbacks_SdlMetal();
                #else
                    IM_ASSERT(false && "Sdl backend is not available!");
                #endif
            }
        #else
            IM_ASSERT(false && "Metal backend is not available!");
        #endif
    }
    else if (params.rendererBackendType == RendererBackendType::Vulkan)
    {
        #ifdef HELLOIMGUI_HAS_VULKAN
            // Choose between CreateBackendCallbacks_GlfwVulkan & CreateBackendCallbacks_SdlVulkan
            if (params.platformBackendType == PlatformBackendType::Glfw)
            {
                #ifdef HELLOIMGUI_USE_GLFW3
                        mRenderingBackendCallbacks = CreateBackendCallbacks_GlfwVulkan();
                #else
                        IM_ASSERT(false && "Glfw3 backend is not available!");
                #endif
            }
            else if (params.platformBackendType == PlatformBackendType::Sdl)
            {
                #ifdef HELLOIMGUI_USE_SDL2
                        mRenderingBackendCallbacks = CreateBackendCallbacks_SdlVulkan();
                #else
                        IM_ASSERT(false && "Sdl backend is not available!");
                #endif
            }
        #else
            IM_ASSERT(false && "Vulkan backend is not available!");
        #endif
    }
    else if (params.rendererBackendType == RendererBackendType::DirectX11)
    {
        #ifdef HELLOIMGUI_HAS_DIRECTX11
            // Choose between CreateBackendCallbacks_GlfwDx11 & CreateBackendCallbacks_SdlDx11
            if (params.platformBackendType == PlatformBackendType::Glfw)
            {
                    #ifdef HELLOIMGUI_USE_GLFW3
                        mRenderingBackendCallbacks = CreateBackendCallbacks_GlfwDx11();
                    #else
                        IM_ASSERT(false && "Glfw3 backend is not available!");
                    #endif
            }
            else if (params.platformBackendType == PlatformBackendType::Sdl)
            {
                    #ifdef HELLOIMGUI_USE_SDL2
                        mRenderingBackendCallbacks = CreateBackendCallbacks_SdlDx11();
                    #else
                        IM_ASSERT(false && "Sdl backend is not available!");
                    #endif
            }
        #else
            IM_ASSERT(false && "DirectX11 backend is not available!");
        #endif
    }
    else if (params.rendererBackendType == RendererBackendType::DirectX12)
    {
        #ifdef HELLOIMGUI_HAS_DIRECTX12
            // Choose between CreateBackendCallbacks_GlfwDx12 & CreateBackendCallbacks_SdlDx12
            if (params.platformBackendType == PlatformBackendType::Glfw)
            {
                    #ifdef HELLOIMGUI_USE_GLFW3
                        // mRenderingBackendCallbacks = CreateBackendCallbacks_GlfwDx12();
                        IM_ASSERT(false && "The combination GLFW3 + Dx12 is not implemented!");
                    #else
                        IM_ASSERT(false && "Glfw3 backend is not available!");
                    #endif
            }
            else if (params.platformBackendType == PlatformBackendType::Sdl)
            {
                    #ifdef HELLOIMGUI_USE_SDL2
                        mRenderingBackendCallbacks = CreateBackendCallbacks_SdlDx12();
                    #else
                        IM_ASSERT(false && "Sdl backend is not available!");
                    #endif
            }
        #else
            IM_ASSERT(false && "DirectX12 backend is not available!");
        #endif
    }
    else if (params.rendererBackendType == RendererBackendType::Null)
    {
        mRenderingBackendCallbacks = CreateBackendCallbacks_Null();
    }
    else
    {
        fprintf(stderr, "Missing rendering backend! %s\n", gMissingBackendErrorMessage.c_str());
        IM_ASSERT(false && "Bad rendering backend type!");
    }
}


void AbstractRunner::Setup()
{
    auto& self = *this;
    InitRenderBackendCallbacks();

    InitImGuiContext();
    SetImGuiPrefs();

    // Init platform backend (SDL, Glfw)
    Impl_InitPlatformBackend();

    #ifdef HELLOIMGUI_HAS_OPENGL
        if (params.rendererBackendType == RendererBackendType::OpenGL3)
            Impl_Select_Gl_Version();
    #endif

    PrepareWindowGeometry();

    auto fnRenderCallbackDuringResize = [this]()
    {
        if (! mWasWindowResizedByCodeDuringThisFrame)
        {
            //printf("Window resized by user\n");
            CreateFramesAndRender(true);
        }
        else
        {
            //printf("Window resized by code\n");
        }
    };

    Impl_CreateWindow(fnRenderCallbackDuringResize);

    #ifdef HELLOIMGUI_HAS_OPENGL
        if (params.rendererBackendType == RendererBackendType::OpenGL3)
        {
            Impl_CreateGlContext();
            Impl_InitGlLoader();
        }
    #endif

    Impl_SetWindowIcon();

    // The order is important: first read the DPI aware params
    SetupDpiAwareParams();
    // Then adjust window size if needed
    AdjustWindowBoundsAfterCreation_IfDpiChangedBetweenRuns();


    // This should be done before Impl_LinkPlatformAndRenderBackends()
    // because, in the case of glfw ImGui_ImplGlfw_InstallCallbacks
    // will chain the user callbacks with ImGui callbacks;
	// and PostInit() is a good place for the user to install callbacks
    if (params.callbacks.PostInit_AddPlatformBackendCallbacks)
        params.callbacks.PostInit_AddPlatformBackendCallbacks();

    Impl_LinkPlatformAndRenderBackends();

    if (params.callbacks.PostInit)
        params.callbacks.PostInit();

    params.callbacks.SetupImGuiConfig();

    #ifdef HELLOIMGUI_WITH_TEST_ENGINE
        if (params.useImGuiTestEngine)
            TestEngineCallbacks::Setup();
    #endif

    //
    // load fonts & set ImGui::GetIO().FontGlobalScale
    //

    // LoadAdditionalFonts will load fonts and resize them by 1./FontGlobalScale
    // (if and only if it uses HelloImGui::LoadFontTTF instead of ImGui's font loading functions)
    ImGui::GetIO().Fonts->Clear();
    params.callbacks.LoadAdditionalFonts();
    params.callbacks.LoadAdditionalFonts = nullptr;
    bool buildSuccess = ImGui::GetIO().Fonts->Build();
    IM_ASSERT(buildSuccess && "ImGui::GetIO().Fonts->Build() failed!");
    {
        // Reset FontGlobalScale if we did not use HelloImGui font loading mechanism
        if (! HelloImGui::DidCallHelloImGuiLoadFontTTF())
        {
            float dpiFactor = mBackendWindowHelper->GetWindowSizeDpiScaleFactor(mWindow);
            ImGui::GetIO().FontGlobalScale = dpiFactor;
        }
    }

    DockingDetails::ConfigureImGuiDocking(params.imGuiWindowParams);
    HelloImGuiIniSettings::LoadHelloImGuiMiscSettings(IniSettingsLocation(params), &params);
    SetLayoutResetIfNeeded();

    ImGuiTheme::ApplyTweakedTheme(params.imGuiWindowParams.tweakedTheme);

    // Fix issue with ImGui & Viewports: title bar cannot be transparent
    if (params.imGuiWindowParams.enableViewports)
    {
        auto& style = ImGui::GetStyle();
        style.Colors[ImGuiCol_TitleBg].w = 1.f;
        style.Colors[ImGuiCol_TitleBgActive].w = 1.f;
        style.Colors[ImGuiCol_TitleBgCollapsed].w = 1.f;
    }
    params.callbacks.SetupImGuiStyle();

    // Create a remote display handler if needed
    mRemoteDisplayHandler.Create();
    mRemoteDisplayHandler.SendFonts();
}


void AbstractRunner::SetLayoutResetIfNeeded()
{
    if (params.imGuiWindowParams.defaultImGuiWindowType == DefaultImGuiWindowType::ProvideFullScreenDockSpace)
    {
        if (params.dockingParams.layoutCondition == DockingLayoutCondition::FirstUseEver)
        {
            if (!HelloImGuiIniSettings::HasUserDockingSettingsInImguiSettings(IniSettingsLocation(params), params.dockingParams))
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
    DockingDetails::ShowToolbars(params);
    if (params.imGuiWindowParams.showMenuBar)
        Menu_StatusBar::ShowMenu(params);

    DockingDetails::ProvideWindowOrDock(params);

    if (params.appWindowParams.borderless) // Need to add params.appWindowParams.borderlessResizable
    {
#if !defined(HELLOIMGUI_MOBILEDEVICE) && !defined(__EMSCRIPTEN__)
        mWasWindowResizedByCodeDuringThisFrame = false;
        auto fnGetWindowBounds = [this]() -> ScreenBounds
        {
            return mBackendWindowHelper->GetWindowBounds(mWindow);
        };
        auto fnSetWindowBounds = [this](const ScreenBounds& bounds)
        {
            setWasWindowResizedByCodeDuringThisFrame();
            mBackendWindowHelper->SetWindowBounds(mWindow, bounds);
        };
        bool shouldClose = HandleBorderlessMovable(fnGetWindowBounds, fnSetWindowBounds, params);
        if (shouldClose)
            params.appShallExit = true;
#endif
    }

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
            mGeometryHelper->TrySetWindowSize(
                mBackendWindowHelper.get(), mWindow, userWidgetsSize,
                this->setWasWindowResizedByCodeDuringThisFrame);
        }
    }

    DockingDetails::ShowDockableWindows(params.dockingParams.dockableWindows);

    if (params.imGuiWindowParams.showStatusBar)
        Menu_StatusBar::ShowStatusBar(params);

    ShowThemeTweakGuiWindow_Static();

    if (params.callbacks.PostRenderDockableWindows)
        params.callbacks.PostRenderDockableWindows();

    DockingDetails::CloseWindowOrDock(params.imGuiWindowParams);
}


void _UpdateFrameRateStats(); // See hello_imgui.cpp


void AbstractRunner::CreateFramesAndRender(bool insideReentrantCall)
{
    // Note: the reentrant call is non-existent for most cases:
    // The preference
    //    repaintDuringResize_GotchaReentrantRepaint
    // is false by default. As the name suggests, it is reserved for advanced users
    // who are ready to deal with the possible consequences.
    //
    // This reentrant call is used to be able to repaint during window resize.
    // It works on some combinations of OS / platform / rendering backends, but not all.
    // See https://github.com/pthom/hello_imgui/issues/112
    if (insideReentrantCall && ! params.appWindowParams.repaintDuringResize_GotchaReentrantRepaint)
        return;

    // ======================================================================================
    //                         Introduction - Lambdas definitions
    //
    //    We only define lambdas here. They are called later in this method
    //
    // ======================================================================================

    //
    // Some lambdas may call user callbacks, and are named with a suffix "UserCallback".
    // When running in python, they *need* to have the GIL!
    //

    // Reload fonts if DPI scale changed
    auto fnReloadFontsIfDpiScaleChanged = [this]()
    {
        if (CheckDpiAwareParamsChanges())
        {
            if (_reloadAllDpiResponsiveFonts())
            {
                DpiLog("_CheckDpiAwareParamsChanges returned true => reloaded all fonts\n");
                // cf https://github.com/ocornut/imgui/issues/6547: we need to recreate the rendering backend device objects
                mRenderingBackendCallbacks->Impl_DestroyFontTexture();
                mRenderingBackendCallbacks->Impl_CreateFontTexture();
                mRemoteDisplayHandler.SendFonts();
            }
        }
    };

    auto fnHandleLayout = [this]()
    {
        LayoutSettings_HandleChanges();

#if TARGET_OS_IOS
        auto insets = GetIPhoneSafeAreaInsets();
        params.appWindowParams.edgeInsets.top = insets.top;
        params.appWindowParams.edgeInsets.left = insets.left;
        params.appWindowParams.edgeInsets.bottom = insets.bottom;
        params.appWindowParams.edgeInsets.right = insets.right;
#endif
    };

    auto fnRegisterTests_UserCallback = [this]()
    {
        #ifdef HELLOIMGUI_WITH_TEST_ENGINE
        // This block calls a user callback, so it cannot be inside SCOPED_RELEASE_GIL_ON_MAIN_THREAD
        if (mIdxFrame >= 1)
        {
            if (params.useImGuiTestEngine && params.callbacks.RegisterTests && (!params.callbacks.registerTestsCalled))
            {
                params.callbacks.RegisterTests();
                params.callbacks.registerTestsCalled = true;
            }
        }
        #endif
    };


    // handle window size and position on first frames
    auto fnHandleWindowSizeAndPositionOnFirstFrames_AndAfterResize = [this]()
    {
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
            mGeometryHelper->EnsureWindowFitsMonitor(mBackendWindowHelper.get(), mWindow, this->setWasWindowResizedByCodeDuringThisFrame);

            // if this is the third frame, and the user wanted a centered window, let's recenter it
            // we do this on the third frame (mIdxFrame == 2), since the initial autosize happens on the second
            // (see WantAutoSize())
            if (params.appWindowParams.windowGeometry.positionMode == HelloImGui::WindowPositionMode::MonitorCenter &&
                (mIdxFrame == 2))
                mGeometryHelper->CenterWindowOnMonitor(mBackendWindowHelper.get(), mWindow, this->setWasWindowResizedByCodeDuringThisFrame);

            mWasWindowAutoResizedOnPreviousFrame = false;
            params.appWindowParams.windowGeometry.resizeAppWindowAtNextFrame = false;
        }


        // v/   At the 4th frame (mIdxFrame >= 3), we finally show the window
        if (mIdxFrame == 3)
        {
            if (params.appWindowParams.hidden)
                mBackendWindowHelper->HideWindow(mWindow);
            else
                mBackendWindowHelper->ShowWindow(mWindow);
            gStatics.lastHiddenState = params.appWindowParams.hidden;
        }
        // On subsequent frames, we take into account user modifications of appWindowParams.hidden
        if (mIdxFrame > 3)
        {
            if (params.appWindowParams.hidden != gStatics.lastHiddenState)
            {
                gStatics.lastHiddenState = params.appWindowParams.hidden;
                if (params.appWindowParams.hidden)
                    mBackendWindowHelper->HideWindow(mWindow);
                else
                    mBackendWindowHelper->ShowWindow(mWindow);
            }
        }

        // Transmit window size to remote server (if needed)
        #ifdef HELLOIMGUI_WITH_REMOTE_DISPLAY
        if ((mIdxFrame > 3) && params.remoteParams.transmitWindowSize)
        {
            auto windowSize = params.appWindowParams.windowGeometry.size;
            mRemoteDisplayHandler.TransmitWindowSizeToDisplay(windowSize);
        }
        #endif
    };


    // Handle idling & poll events
    // Warning:
    // Due to severe gotcha inside GLFW and SDL: PollEvent is supposed to
    // return immediately, but it doesn't when resizing the window!
    // Instead, you have to subscribe to a kind of special "mid-resize" event,
    // and then call the render function yourself.
    // As a consequence, this function is not called inside reentrant calls
    //
    // Under emscripten, the idling implementation is different:
    // we cannot sleep (which would lead to a busy wait), so we skip rendering the frame if needed
    // => we return true if we should not render this frame
    auto fnHandleIdlingAndPollEvents_MayReRenderDuringResize_GotchaReentrant = [this]()-> bool
    {
        // This will be our return value: can only be true under emscripten, when idling is needed
        bool shallSkipRenderingThisFrame = false;

        double now = Internal::ClockSeconds();

        auto fnShallIdle = [&]() -> bool
        {
            assert(params.fpsIdling.fpsIdle >= 0.f && "fpsIdle must be >= 0");

            // If the last event is recent, do not idle
            bool hasRecentEvent = (now - gStatics.timeLastEvent) < (double)params.fpsIdling.timeActiveAfterLastEvent;
            // If idling is disabled by params, do not idle
            bool isIdlingDisabledByParams = (! params.fpsIdling.enableIdling || (params.fpsIdling.fpsIdle <= 0.f) );

            // If the test engine is running, do not idle
            bool isTestEngineRunning = false;
            #ifdef HELLOIMGUI_WITH_TEST_ENGINE
            if (params.useImGuiTestEngine && TestEngineCallbacks::IsRunningTest())
                isTestEngineRunning = true;
            #endif

            // If the app started recently, do not idle
            bool startedRecently = mIdxFrame < 12;

            bool preventIdling = isIdlingDisabledByParams || hasRecentEvent || isTestEngineRunning || ShouldRemoteDisplay() || startedRecently;
            return ! preventIdling;
        };

        bool shallIdle = fnShallIdle();
        params.fpsIdling.isIdling = shallIdle;

        // Keep track of the time of the last event,
        // by counting the number of events in the input queue
        int nbEventsBefore = ImGui::GetCurrentContext()->InputEventsQueue.size();

        #ifndef __EMSCRIPTEN__
        // Idling for non emscripten, where HelloImGui is responsible for the main loop.
        // This form of idling will call WaitForEventTimeout(), which may call sleep():
        if (shallIdle)
        {
            double waitTimeout = 1. / (double) params.fpsIdling.fpsIdle;
            mBackendWindowHelper->WaitForEventTimeout(waitTimeout);
        }
        #endif

        // Poll Events (this fills GImGui.InputEventsQueue)
        // May re-trigger a full call to CreateFramesAndRender(skipPollEvents=true)
        // if we are in the middle of a window resize!!!
        // (There is a gotcha in GLFW and SDL, where PollEvent is blocking during a window resize,
        //  which is circumvented by calling CreateFramesAndRender(skipPollEvents=true)) in a specific callback
        //  defined during the window creation)
        Impl_PollEvents();

        int nbEventsAfter = ImGui::GetCurrentContext()->InputEventsQueue.size();
        if (nbEventsAfter > nbEventsBefore)
            gStatics.timeLastEvent = now;

#ifdef __EMSCRIPTEN__
        // Idling for emscripten: we cannot sleep, so we skip rendering the frame if needed
        bool wasLastFrameRenderedInTimeForDesiredFps = ((now - gStatics.lastRefreshTime) < 1. / params.fpsIdling.fpsIdle);
        shallSkipRenderingThisFrame = shallIdle && wasLastFrameRenderedInTimeForDesiredFps;
#endif

        if (! shallSkipRenderingThisFrame)
            gStatics.lastRefreshTime = now;
        return shallSkipRenderingThisFrame;
    };


    // Load additional fonts during execution
    auto fnLoadAdditionalFontDuringExecution_UserCallback = [this]()
    {
        // This block calls a user callback, so it cannot be inside SCOPED_RELEASE_GIL_ON_MAIN_THREAD
        if (params.callbacks.LoadAdditionalFonts != nullptr)
        {
            params.callbacks.LoadAdditionalFonts();
            ImGui::GetIO().Fonts->Build();
            // cf https://github.com/ocornut/imgui/issues/6547
            // We need to recreate the rendering backend device objects
            mRenderingBackendCallbacks->Impl_DestroyFontTexture();
            mRenderingBackendCallbacks->Impl_CreateFontTexture();
            params.callbacks.LoadAdditionalFonts = nullptr;

            mRemoteDisplayHandler.SendFonts();
        }
    };

    //
    // Rendering logic
    //

    // New Frame / Rendering and Platform Backend (not ImGui)
    auto fnNewFrameRenderingAndPlatformBackend = [this]()
    {
        mRenderingBackendCallbacks->Impl_NewFrame_3D();
        Impl_NewFrame_PlatformBackend();

        {
            // Workaround against SDL clock that sometimes leads to io.DeltaTime=0.f on emscripten
            // (which fails to an `IM_ASSERT(io.DeltaTime) > 0` in ImGui::NewFrame())
            //
            // This can be removed once the commit 07490618 was merged into imgui docking branch
            // (see https://github.com/ocornut/imgui/commit/07490618ae47fdb9f625565fbb183593170a6a72)
            auto &io = ImGui::GetIO();
            if (io.DeltaTime <= 0.f)
                io.DeltaTime = 1.f / 60.f;
        }

    };


    // check potential OpenGL error on first frame, that may be due to a font loading error
    auto fnCheckOpenGlErrorOnFirstFrame_WarnPotentialFontError = [this]()
    {
        #ifdef HELLOIMGUI_HAS_OPENGL
        if (params.rendererBackendType == RendererBackendType::OpenGL3)
        {
            if (mIdxFrame == 0)
            {
                auto error = glGetError();
                if (error != 0)
                {
                    fprintf(stderr, "OpenGL error detected on first frame: %d. May be the font texture is too big\n", error);
                }
            }
        }
        #endif
    };


    auto fnDrawCustomBackgroundOrClearColor_UserCallback = [this]()
    {
        // This block calls a user callback, so it cannot be inside SCOPED_RELEASE_GIL_ON_MAIN_THREAD
        // CustomBackground is a user callback
        if (params.callbacks.CustomBackground)
            params.callbacks.CustomBackground();
        else
            mRenderingBackendCallbacks->Impl_Frame_3D_ClearColor(params.imGuiWindowParams.backgroundColor);
    };


    // Render and Swap
    auto fnRenderAndSwap = [this]()
    {
        ImGui::Render();
        mRenderingBackendCallbacks->Impl_RenderDrawData_To_3D();

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            Impl_UpdateAndRenderAdditionalPlatformWindows();

        Impl_SwapBuffers();

        mRemoteDisplayHandler.Heartbeat_PostImGuiRender();
    };

    // fnRenderGui_UserCallback calls RenderGui(), which may call many user callbacks
    auto fnRenderGui_UserCallback = [this]()
    {
         RenderGui();
    };

    auto fnCallTestEngineCallbackPostSwap = [this]()
    {
#ifdef HELLOIMGUI_WITH_TEST_ENGINE
        // TestEngineCallbacks::PostSwap() handles the GIL in its own way,
        // it can not be called inside SCOPED_RELEASE_GIL_ON_MAIN_THREAD
        if (params.useImGuiTestEngine)
        {
            TestEngineCallbacks::PostSwap();
        }
#endif
    };

    // ======================================================================================
    //                         Real work - Lambdas calls
    //
    //    Below, we call the lambdas defined above
    //
    // ======================================================================================
    //
    // They are two important gotchas to know here
    // 1. ImGui test engine & coroutine threading
    // 2. Possible reentrance into this function when resizing the window
    //    (see fnHandleIdlingAndPollEvents_MayReRenderDuringResize_GotchaReentrant)
    //
    //
    // 1. Gotcha due to the integration of ImGui Test Engine in Python:
    // ----------------------------------------------------------------
    //   Within ImGui test engine there are two threads!
    //   One thread is the main thread, and the second is the "coroutine" thread.
    //   They run like a coroutine and actually will never run in parallel!
    //   However, when running Python some python code may be called
    //      - in the main thread (standard user callbacks)
    //      - in the coroutine thread (ImGui test engine callbacks)!
    //
    // We have to be very, very careful about passing correctly the GIL between threads,
    // since Python needs the GIL to be acquired on the correct thread
    //
    // Some blocks below include a SCOPED_RELEASE_GIL_ON_MAIN_THREAD
    // This means that they will release the GIL on the main thread,
    // allowing for a potential execution of Python on the coroutine thread.
    //
    // Inside these blocks, it is strictly forbidden to call any user callback
    // (since they might run Python code on the main thread)
    //
    //    // For more details, see
    //    //     external/imgui_test_engine/imgui_test_engine/imgui_test_engine/imgui_te_python_gil.jpg
    //
    // Also, two ImGui methods handle the test engine and its coroutine + thread switches:
    //  => They should not be in a block SCOPED_RELEASE_GIL_ON_MAIN_THREAD
    //      ImGui::NewFrame();
    //      TestEngineCallbacks::PostSwap
    // And fnHandleIdlingAndPollEvents_MayReRenderDuringResize_GotchaReentrant
    // shall also not be in a block SCOPED_RELEASE_GIL_ON_MAIN_THREAD
    // (see second gotcha below)
    //
    //
    // 2. Gotcha / possible re-entrance into this function when resizing the window
    // -----------------------------------------------------------------------------
    // See https://github.com/pthom/hello_imgui/issues/112
    // There is a severe gotcha inside GLFW and SDL: PollEvent is supposed to
    // return immediately, but it doesn't when resizing the window!
    // If you do nothing, the window content is "stretching" during the resize
    // but not update is done.
    // Instead, you have to subscribe to a kind of special "mid-resize" event,
    // and then call the render function yourself.
    //
    // See IBackendWindowHelper::CreateWindow(AppWindowParams &info, const BackendOptions& backendOptions,
    //                                           std::function<void()> renderCallbackDuringResize) = 0;
    // Where renderCallbackDuringResize is set to CreateFramesAndRender(skipPollEvents=true)

    // Will display on remote server if needed
    mRemoteDisplayHandler.Heartbeat_PreImGuiNewFrame();

    if (ShouldRemoteDisplay()) // FIXME, unrelated
    {
        // if displaying remote, the FPS is limited on the server to a value between 30 and 60 fps
        // We cannot idle too slow, other the GUI becomes really sluggish
        // Ideally, we should ask the server about it current refresh rate
        // (At the moment, we can only deliver 30fps)
        params.fpsIdling.fpsIdle = 30.f;
    }

    {
        SCOPED_RELEASE_GIL_ON_MAIN_THREAD;
        fnReloadFontsIfDpiScaleChanged();
        fnHandleLayout();
    }


    fnRegisterTests_UserCallback();

    {
        SCOPED_RELEASE_GIL_ON_MAIN_THREAD;
        fnHandleWindowSizeAndPositionOnFirstFrames_AndAfterResize();
    }

    // Handle idling & poll events
    // Warning:
    // Due to severe gotcha inside GLFW and SDL: PollEvent is supposed to
    // return immediately, but it doesn't when resizing the window!
    // Instead, you have to subscribe to a kind of special "mid-resize" event,
    // and then call the render function yourself.
    if (!insideReentrantCall)  // Do not poll events again in a reentrant call!
    {
        // We cannot release the GIL here, since we may have a reentrant call!
        bool shallSkipThisFrame = fnHandleIdlingAndPollEvents_MayReRenderDuringResize_GotchaReentrant();
        if (shallSkipThisFrame)
        {
            mIdxFrame += 1;
            return;
        }
    }

    {
        _UpdateFrameRateStats(); // not in a SCOPED_RELEASE_GIL_ON_MAIN_THREAD, because it is very fast
        fnLoadAdditionalFontDuringExecution_UserCallback(); // User callback
    }


    if ((params.callbacks.PreNewFrame) && !insideReentrantCall)
        params.callbacks.PreNewFrame();

    {
        SCOPED_RELEASE_GIL_ON_MAIN_THREAD;
        fnNewFrameRenderingAndPlatformBackend();
    }

    // ImGui::NewFrame may call ImGuiTestEngine_PostNewFrame, which in turn handles the GIL in its own way,
    // so that it can *NOT* be called inside SCOPED_RELEASE_GIL_ON_MAIN_THREAD
    ImGui::NewFrame();

    {
        fnCheckOpenGlErrorOnFirstFrame_WarnPotentialFontError(); // not in a SCOPED_RELEASE_GIL_ON_MAIN_THREAD, because it is very fast and rare

        fnDrawCustomBackgroundOrClearColor_UserCallback(); // User callback
    }

    // iii/ At the end of the second frame, we measure the size of the widgets and use it as the application window size,
    // if the user required auto size
    // ==> Note: RenderGui() may measure the size of the window and resize it if mIdxFrame==1
    // RenderGui may call many user callbacks, so it should not be inside SCOPED_RELEASE_GIL_ON_MAIN_THREAD
    fnRenderGui_UserCallback();

    if (params.callbacks.BeforeImGuiRender)
        params.callbacks.BeforeImGuiRender();

    {
        SCOPED_RELEASE_GIL_ON_MAIN_THREAD;
        fnRenderAndSwap();
    }

    // AfterSwap is a user callback, so it should not be inside SCOPED_RELEASE_GIL_ON_MAIN_THREAD
    if (params.callbacks.AfterSwap)
        params.callbacks.AfterSwap();

    // TestEngineCallbacks::PostSwap() handles the GIL in its own way,
    // it can not be called inside SCOPED_RELEASE_GIL_ON_MAIN_THREAD
    fnCallTestEngineCallbackPostSwap();

    if (!mRemoteDisplayHandler.CanQuitApp())
        params.appShallExit = false;

    mIdxFrame += 1;
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
            HelloImGuiIniSettings::SaveLastRunWindowBounds(IniSettingsLocation(params),
                                                           mBackendWindowHelper->GetWindowBounds(mWindow));
        LayoutSettings_Save();
        HelloImGuiIniSettings::SaveHelloImGuiMiscSettings(IniSettingsLocation(params), params);
    }

    HelloImGui::internal::Free_ImageFromAssetMap();

    if (!gotException && params.callbacks.BeforeExit)
            params.callbacks.BeforeExit();

    #ifdef HELLOIMGUI_WITH_TEST_ENGINE
        if (params.useImGuiTestEngine)
            TestEngineCallbacks::TearDown_ImGuiContextAlive();
    #endif

    mRenderingBackendCallbacks->Impl_Shutdown_3D();
    Impl_Cleanup();


    if (!gotException && params.callbacks.BeforeExit_PostCleanup)
        params.callbacks.BeforeExit_PostCleanup();
    #ifdef HELLOIMGUI_WITH_TEST_ENGINE
        if (params.useImGuiTestEngine)
            TestEngineCallbacks::TearDown_ImGuiContextDestroyed();
    #endif

    mRemoteDisplayHandler.Shutdown();
}


void AbstractRunner::SaveUserPref(const std::string& userPrefName, const std::string& userPrefContent)
{
    HelloImGuiIniSettings::SaveUserPref(IniSettingsLocation(params), userPrefName, userPrefContent);
}
std::string AbstractRunner::LoadUserPref(const std::string& userPrefName)
{
    return HelloImGuiIniSettings::LoadUserPref(IniSettingsLocation(params), userPrefName);
}

bool AbstractRunner::ShouldRemoteDisplay()
{
    return mRemoteDisplayHandler.ShouldRemoteDisplay();
}



}  // namespace HelloImGui
