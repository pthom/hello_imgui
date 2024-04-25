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


AbstractRunner::AbstractRunner(RunnerParams &params_)
: params(params_) {}


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

        shallSizeRelativeTo96Ppi =
            isStandardSizeMode && isWindowPpiRelativeSize && !doRestorePreviousGeometry;
    }
    return shallSizeRelativeTo96Ppi;
}

void AbstractRunner::ReadOpenGlOptions()
{
    // cf renderer_backend_options.h:
    //
    // OpenGlOptions contains advanced options used at the startup of OpenGL.
    // These parameters are reserved for advanced users.
    // By default, Hello ImGui will select reasonable default values, and these parameters are not used.
    // Use at your own risk, as they make break the multi-platform compatibility of your application!
    // All these parameters are platform dependent.
    // For real multiplatform examples, see
    //     hello_imgui/src/hello_imgui/internal/backend_impls/opengl_setup_helper/opengl_setup_glfw.cpp
    // and
    //     hello_imgui/src/hello_imgui/internal/backend_impls/opengl_setup_helper/opengl_setup_sdl.cpp
    //
    // How to set those values manually:
    // ---------------------------------
    // you may set them manually:
    //    (1) Either by setting them programmatically in your application
    //        (set their values in `runnerParams.rendererBackendOptions.openGlOptions`)
    //    (2) Either by setting them in a `hello_imgui.ini` file in the current folder, or any of its parent folders.
    //       (this is useful when you want to set them for a specific app or set of apps, without modifying the app code)
    // Note: if several methods are used, the order of priority is (1) > (2)
    //
    // Example content of a hello_imgui.ini file:
    // ------------------------------
    //    [OpenGlOptions]
    //    GlslVersion = 130
    //    MajorVersion = 3
    //    MinorVersion = 2
    //    UseCoreProfile = true
    //    UseForwardCompat = false

    // If options specified by the program, do not read them.
    if (params.rendererBackendOptions.openGlOptions.has_value())
        return;

    std::string openGlSection = "OpenGlOptions";

    auto majorVersion = HelloImGuiIniAnyParentFolder::readIntValue(openGlSection, "MajorVersion");
    auto minorVersion = HelloImGuiIniAnyParentFolder::readIntValue(openGlSection, "MinorVersion");
    auto useCoreProfile = HelloImGuiIniAnyParentFolder::readBoolValue(openGlSection, "UseCoreProfile");
    auto useForwardCompat = HelloImGuiIniAnyParentFolder::readBoolValue(openGlSection, "UseForwardCompat");
    auto glslVersion = HelloImGuiIniAnyParentFolder::readStringValue(openGlSection, "GlslVersion");

    bool isAnySettingsSpecified = majorVersion.has_value() || minorVersion.has_value() || useCoreProfile.has_value() || useForwardCompat.has_value() || glslVersion.has_value();

    if (isAnySettingsSpecified)
    {
        params.rendererBackendOptions.openGlOptions = OpenGlOptions();
        auto& openGlOptions = params.rendererBackendOptions.openGlOptions.value();
        if (majorVersion.has_value())
            openGlOptions.MajorVersion = majorVersion.value();
        if (minorVersion.has_value())
            openGlOptions.MinorVersion = minorVersion.value();
        if (useCoreProfile.has_value())
            openGlOptions.UseCoreProfile = useCoreProfile.value();
        if (useForwardCompat.has_value())
            openGlOptions.UseForwardCompat = useForwardCompat.value();
        if (glslVersion.has_value())
            openGlOptions.GlslVersion = glslVersion.value();
    }

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
    static std::string lastLoadedLayout;
    if (params.dockingParams.layoutName != lastLoadedLayout)
    {
        LayoutSettings_Load();
        lastLoadedLayout = params.dockingParams.layoutName;
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
        #ifdef HELLOIMGUI_USE_NULL
            mRenderingBackendCallbacks = CreateBackendCallbacks_Null();
        #else
            IM_ASSERT(false && "Null backend is not available!");
        #endif
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

    #ifdef HELLOIMGUI_HAS_OPENGL
    ReadOpenGlOptions();
    #endif

    // Init platform backend (SDL, Glfw)
    Impl_InitPlatformBackend();

    #ifdef HELLOIMGUI_HAS_OPENGL
        if (params.rendererBackendType == RendererBackendType::OpenGL3)
            Impl_Select_Gl_Version();
    #endif

    PrepareWindowGeometry();
    Impl_CreateWindow();

    #ifdef HELLOIMGUI_HAS_OPENGL
        if (params.rendererBackendType == RendererBackendType::OpenGL3)
        {
            Impl_CreateGlContext();
            Impl_InitGlLoader();
        }
    #endif

    Impl_SetWindowIcon();

    SetupDpiAwareParams();

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
        bool shouldClose = HandleBorderlessMovable(mWindow, mBackendWindowHelper.get(), params);
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


void _UpdateFrameRateStats(); // See hello_imgui.cpp


void AbstractRunner::CreateFramesAndRender()
{
    // Notes:
    // - this method is rather long, but this is intentional in order to be able to see the logic flow at a glance
    // - the code is organized in blocks of code that are enclosed by
    //       if (true) // Block explanation
    //       {
    //            ...
    //       }
    //   this enables to limit the scope of each block, and enable to collapses them
    //   when inside an IDE
    // - Some blocks include a SCOPED_RELEASE_GIL_ON_MAIN_THREAD statement, like this:
    //       if (true_gil) // block explanation    (`true_gil` is a synonym for "true")
    //       { // SCOPED_RELEASE_GIL_ON_MAIN_THREAD start
    //           SCOPED_RELEASE_GIL_ON_MAIN_THREAD;       // SCOPED_RELEASE_GIL_ON_MAIN_THREAD does nothing if not using python
    //
    //          ... // Block logic that is executed *whenever using Python or not*
    //
    //       } // SCOPED_RELEASE_GIL_ON_MAIN_THREAD end
    //    This means that they will release the Gil for python, and that they should not call
    //    any user callback (which may call python functions)
    // -

    // `foldable_region` is a synonym for "true" (whenever using Python or not using Python)
    // (it is here only to make the code more readable, and to enable to collapse blocks of code)
    constexpr bool foldable_region = true;

    // Will display on remote server if needed
    mRemoteDisplayHandler.Heartbeat_PreImGuiNewFrame();

	if (CheckDpiAwareParamsChanges()) // Reload fonts if DPI scale changed
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

    if (foldable_region) // basic layout checks
    { // SCOPED_RELEASE_GIL_ON_MAIN_THREAD start
        SCOPED_RELEASE_GIL_ON_MAIN_THREAD;


        LayoutSettings_HandleChanges();

        #if TARGET_OS_IOS
        auto insets = GetIPhoneSafeAreaInsets();
        params.appWindowParams.edgeInsets.top = insets.top;
        params.appWindowParams.edgeInsets.left = insets.left;
        params.appWindowParams.edgeInsets.bottom = insets.bottom;
        params.appWindowParams.edgeInsets.right = insets.right;
        #endif
    } // SCOPED_RELEASE_GIL_ON_MAIN_THREAD end

    if (foldable_region) // Register tests
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
    }

    if (foldable_region) // handle window size and position on first frames
    { // SCOPED_RELEASE_GIL_ON_MAIN_THREAD start
        SCOPED_RELEASE_GIL_ON_MAIN_THREAD;

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
            if (params.appWindowParams.windowGeometry.positionMode == HelloImGui::WindowPositionMode::MonitorCenter &&
                (mIdxFrame == 2))
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

        // Transmit window size to remote server (if needed)
        #ifdef HELLOIMGUI_WITH_REMOTE_DISPLAY
        if ((mIdxFrame > 3) && params.remoteParams.transmitWindowSize)
        {
            auto windowSize = params.appWindowParams.windowGeometry.size;
            mRemoteDisplayHandler.TransmitWindowSizeToDisplay(windowSize);
        }
        #endif
    }  // SCOPED_RELEASE_GIL_ON_MAIN_THREAD end

    if(foldable_region) // Handle idling
    { // SCOPED_RELEASE_GIL_ON_MAIN_THREAD start
        SCOPED_RELEASE_GIL_ON_MAIN_THREAD;

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
        {
            mIdxFrame += 1;
            return;
        }
        #endif
    } // SCOPED_RELEASE_GIL_ON_MAIN_THREAD end

	if (foldable_region) // Update frame rate stats
	{
		_UpdateFrameRateStats();
		// printf("Render frame %i, fps=%.1f\n", mIdxFrame, HelloImGui::FrameRate());
	}

	if (foldable_region) // Load additional fonts during execution
    {
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
    }

    //
    // Rendering logic
    //
    if (params.callbacks.PreNewFrame)
        params.callbacks.PreNewFrame();

    if (foldable_region)  // New Frame / Rendering and Platform Backend (not ImGui)
    { // SCOPED_RELEASE_GIL_ON_MAIN_THREAD start
        SCOPED_RELEASE_GIL_ON_MAIN_THREAD;

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

    } // SCOPED_RELEASE_GIL_ON_MAIN_THREAD end

    // ImGui::NewFrame may call ImGuiTestEngine_PostNewFrame, which in turn handles the GIL in its own way,
    // so that it can *NOT* be called inside SCOPED_RELEASE_GIL_ON_MAIN_THREAD
    ImGui::NewFrame();

    if (true) // check potential OpenGL error on first frame, that may be due to a font loading error
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
    }

    // CustomBackground is a user callback
    if (params.callbacks.CustomBackground)
        params.callbacks.CustomBackground();
    else
        mRenderingBackendCallbacks->Impl_Frame_3D_ClearColor(params.imGuiWindowParams.backgroundColor);

    // iii/ At the end of the second frame, we measure the size of the widgets and use it as the application window size, if the user required auto size
    // ==> Note: RenderGui() may measure the size of the window and resize it if mIdxFrame==1
    // RenderGui may call many user callbacks, so it should not be inside SCOPED_RELEASE_GIL_ON_MAIN_THREAD
	RenderGui();

    if (params.callbacks.BeforeImGuiRender)
        params.callbacks.BeforeImGuiRender();

    if (foldable_region) // Render and Swap
    { // SCOPED_RELEASE_GIL_ON_MAIN_THREAD start
        SCOPED_RELEASE_GIL_ON_MAIN_THREAD;

        ImGui::Render();
        mRenderingBackendCallbacks->Impl_RenderDrawData_To_3D();

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            Impl_UpdateAndRenderAdditionalPlatformWindows();

        Impl_SwapBuffers();

        mRemoteDisplayHandler.Heartbeat_PostImGuiRender();
    } // SCOPED_RELEASE_GIL_ON_MAIN_THREAD end

    // AfterSwap is a user callback, so it should not be inside SCOPED_RELEASE_GIL_ON_MAIN_THREAD
    if (params.callbacks.AfterSwap)
        params.callbacks.AfterSwap();

    #ifdef HELLOIMGUI_WITH_TEST_ENGINE
        // TestEngineCallbacks::PostSwap() handles the GIL in its own way,
        // it can not be called inside SCOPED_RELEASE_GIL_ON_MAIN_THREAD
        if (params.useImGuiTestEngine)
        {
            TestEngineCallbacks::PostSwap();
        }
    #endif

    if (!mRemoteDisplayHandler.CanQuitApp())
        params.appShallExit = false;

    mIdxFrame += 1;
}

// Idling for non emscripten, where HelloImGui is responsible for the main loop.
// This form of idling will call WaitForEventTimeout(), which may call sleep():
void AbstractRunner::IdleBySleeping()
{
    #ifdef HELLOIMGUI_WITH_TEST_ENGINE
        if (params.useImGuiTestEngine && TestEngineCallbacks::IsRunningTest())
            return;
	#endif

	if (ShouldRemoteDisplay())
	{
		// if displaying remote, the FPS is limited on the server to a value between 30 and 60 fps
		// We cannot idle too slow, other the GUI becomes really sluggish
		// Ideally, we should ask the server about it current refresh rate
		// (At the moment, we can only deliver 30fps)
		params.fpsIdling.fpsIdle = 30.f;
	}

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
    #ifdef HELLOIMGUI_WITH_TEST_ENGINE
        if (params.useImGuiTestEngine && TestEngineCallbacks::IsRunningTest())
            return false;
    #endif

    ImGuiContext& g = *GImGui;
    bool hasInputEvent =  ! g.InputEventsQueue.empty();

    if (! params.fpsIdling.enableIdling || (params.fpsIdling.fpsIdle <= 0.f) )
    {
        params.fpsIdling.isIdling = false;
        return false;
    }

    static double lastRefreshTime = 0.;
    double now = Internal::ClockSeconds();

    bool shallIdleThisFrame;
    {
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
            HelloImGuiIniSettings::SaveLastRunWindowBounds(IniSettingsLocation(params),
                                                           mBackendWindowHelper->GetWindowBounds(mWindow));
        LayoutSettings_Save();
        HelloImGuiIniSettings::SaveHelloImGuiMiscSettings(IniSettingsLocation(params), params);
    }

    HelloImGui::internal::Free_ImageFromAssetMap();

    if (params.callbacks.BeforeExit)
        params.callbacks.BeforeExit();
    #ifdef HELLOIMGUI_WITH_TEST_ENGINE
        if (params.useImGuiTestEngine)
            TestEngineCallbacks::TearDown_ImGuiContextAlive();
    #endif

    mRenderingBackendCallbacks->Impl_Shutdown_3D();
    Impl_Cleanup();


    if (params.callbacks.BeforeExit_PostCleanup)
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
