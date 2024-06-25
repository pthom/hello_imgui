#pragma once
#include "hello_imgui/app_window_params.h"
#include "hello_imgui/imgui_window_params.h"
#include "hello_imgui/runner_callbacks.h"
#include "hello_imgui/docking_params.h"
#include "hello_imgui/backend_pointers.h"
#include "hello_imgui/remote_params.h"
#include "hello_imgui/renderer_backend_options.h"
#include "hello_imgui/dpi_aware.h"
#include <vector>

namespace HelloImGui
{

// --------------------------------------------------------------------------------------------------------------------

// @@md#PlatformBackendType

// You can select the platform backend type (SDL, GLFW) and the rendering backend type
// via RunnerParams.platformBackendType and RunnerParams.renderingBackendType.

// Platform backend type (SDL, GLFW)
// They are listed in the order of preference when FirstAvailable is selected.
enum class PlatformBackendType
{
    FirstAvailable,
    Glfw,
    Sdl,
    Null
};

// Rendering backend type (OpenGL3, Metal, Vulkan, DirectX11, DirectX12)
// They are listed in the order of preference when FirstAvailable is selected.
enum class RendererBackendType
{
    FirstAvailable,
    OpenGL3,
    Metal,
    Vulkan,
    DirectX11,
    DirectX12,
    Null
};

// @@md

// --------------------------------------------------------------------------------------------------------------------

// @@md#IniFolderType

// IniFolderType is an enum which describes where is the base path to store
// the ini file for the application settings.
//
// You can use IniFolderLocation(iniFolderType) to get the corresponding path.
//
// RunnerParams contains the following members, which are used to compute
// the ini file location:
//     iniFolderType           (IniFolderType::CurrentFolder by default)
//     iniFilename             (empty string by default)
//     iniFilename_useAppWindowTitle
//         (true by default: iniFilename is derived from
//          appWindowParams.windowTitle)
//
// iniFilename may contain a subfolder
// (which will be created inside the iniFolderType folder if needed)
//
enum class IniFolderType
{
    // CurrentFolder: the folder where the application is executed
    // (convenient for development, but not recommended for production)
    CurrentFolder,

    // AbsolutePath: an absolute path
    // (convenient, but not recommended if targeting multiple platforms)
    AbsolutePath,

    // AppUserConfigFolder:
    //      AppData under Windows (Example: C:\Users\[Username]\AppData\Roaming under windows)
    //      ~/.config under Linux
    //      "~/Library/Application Support" under macOS
    // (recommended for production, if settings do not need to be easily accessible by the user)
    AppUserConfigFolder,

    // AppExecutableFolder: the folder where the application executable is located
    // (this may be different from CurrentFolder if the application is launched from a shortcut)
    // (convenient for development, but not recommended for production)
    AppExecutableFolder,

    // HomeFolder: the user home folder
    // (recommended for production, if settings need to be easily accessible by the user)
    HomeFolder,

    // DocumentsFolder: the user documents folder
    DocumentsFolder,

    // TempFolder: the system temp folder
    TempFolder
};

// Returns the path corresponding to the given IniFolderType
std::string IniFolderLocation(IniFolderType iniFolderType);

// @@md

// --------------------------------------------------------------------------------------------------------------------

// @@md#FpsIdling

// FpsIdling is a struct that contains Fps Idling parameters
struct FpsIdling
{
    // `fpsIdle`: _float, default=9_.
    //  ImGui applications can consume a lot of CPU, since they update the screen
    //  very frequently. In order to reduce the CPU usage, the FPS is reduced when
    //  no user interaction is detected.
    //  This is ok most of the time but if you are displaying animated widgets
    //  (for example a live video), you may want to ask for a faster refresh:
    //  either increase fpsIdle, or set it to 0 for maximum refresh speed
    //  (you can change this value during the execution depending on your application
    //  refresh needs)
    float fpsIdle = 9.f;

    // `timeActiveAfterLastEvent`: _float, default=3.f_.
    //  Time in seconds after the last event before the application is considered idling.
    float timeActiveAfterLastEvent = 3.f;

    // `enableIdling`: _bool, default=true_.
    //  Disable idling by setting this to false.
    //  (this can be changed dynamically during execution)
    bool  enableIdling = true;

    // `isIdling`: bool (dynamically updated during execution)
    //  This bool will be updated during the application execution,
    //  and will be set to true when it is idling.
    bool  isIdling = false;

    // `rememberEnableIdling`: _bool, default=true_.
    //  If true, the last value of enableIdling is restored from the settings at startup.
    bool  rememberEnableIdling = false;
};
// @@md


// --------------------------------------------------------------------------------------------------------------------

// @@md#RunnerParams

// RunnerParams contains the settings and callbacks needed to run an application.
//
struct RunnerParams
{
    // --------------- Callbacks and Window params -------------------

    // `callbacks`: _see runner_callbacks.h_
    // callbacks.ShowGui() will render the gui, ShowMenus() will show the menus, etc.
    RunnerCallbacks callbacks;

    // `appWindowParams`: _see app_window_params.h_
    // application Window Params (position, size, title)
    AppWindowParams appWindowParams;

    // `imGuiWindowParams`: _see imgui_window_params.h_
    // imgui window params (use docking, showMenuBar, ProvideFullScreenWindow, etc.)
    ImGuiWindowParams imGuiWindowParams;


    // --------------- Docking -------------------

    // `dockingParams`: _see docking_params.h_
    // dockable windows content and layout
    DockingParams dockingParams;

    // `alternativeDockingLayouts`: _vector<DockingParams>, default=empty_
    // List of possible additional layout for the applications. Only used in advanced
    // cases when several layouts are available.
    std::vector<DockingParams> alternativeDockingLayouts;

    // `rememberSelectedAlternativeLayout`: _bool, default=true_
    // Shall the application remember the last selected layout. Only used in advanced
    // cases when several layouts are available.
    bool rememberSelectedAlternativeLayout = true;


    // --------------- Backends -------------------

    // `backendPointers`: _see backend_pointers.h_
    // A struct that contains optional pointers to the backend implementations.
    // These pointers will be filled when the application starts
    BackendPointers backendPointers;

    // `rendererBackendOptions`: _see renderer_backend_options.h_
    // Options for the renderer backend
    RendererBackendOptions rendererBackendOptions;

    // `platformBackendType`: _enum PlatformBackendType, default=PlatformBackendType::FirstAvailable_
    // Select the wanted platform backend type between `Sdl`, `Glfw`.
    // if `FirstAvailable`, Glfw will be preferred over Sdl when both are available.
    // Only useful when multiple backend are compiled and available.
    PlatformBackendType platformBackendType = PlatformBackendType::FirstAvailable;

    // `renderingBackendType`: _enum RenderingBackendType, default=RenderingBackendType::FirstAvailable_
    // Select the wanted rendering backend type between `OpenGL3`, `Metal`, `Vulkan`, `DirectX11`, `DirectX12`.
    // if `FirstAvailable`, it will be selected in the order of preference mentioned above.
    // Only useful when multiple rendering backend are compiled and available.
    RendererBackendType rendererBackendType = RendererBackendType::FirstAvailable;


    // --------------- Settings -------------------

    // `iniFolderType`: _IniFolderType, default = IniFolderType::CurrentFolder_
    // Sets the folder where imgui will save its params.
    // (possible values are:
    //     CurrentFolder, AppUserConfigFolder, DocumentsFolder,
    //     HomeFolder, TempFolder, AppExecutableFolder)
    // AppUserConfigFolder is
    //     [Home]/AppData/Roaming under Windows,
    //     ~/.config under Linux,
    //     ~/Library/Application Support under macOS
    IniFolderType iniFolderType = IniFolderType::CurrentFolder;
    // `iniFilename`: _string, default = ""_
    // Sets the ini filename under which imgui will save its params.
    // Its path is relative to the path given by iniFolderType, and can include
    // a subfolder (which will be created if needed).
    // If iniFilename empty, then it will be derived from
    // appWindowParams.windowTitle
    // (if both are empty, the ini filename will be imgui.ini).
    std::string iniFilename = "";  // relative to iniFolderType
    // `iniFilename_useAppWindowTitle`: _bool, default = true_.
    // Shall the iniFilename be derived from appWindowParams.windowTitle (if not empty)
    bool iniFilename_useAppWindowTitle = true;


    // --------------- Exit -------------------

    // * `appShallExit`: _bool, default=false_.
    // During execution, set this to true to exit the app.
    // _Note: 'appShallExit' has no effect on Mobile Devices (iOS, Android)
    // and under emscripten, since these apps shall not exit._
    bool appShallExit = false;

    // --------------- Idling -------------------

    // `fpsIdling`: _FpsIdling_. Idling parameters
    // (set fpsIdling.enableIdling to false to disable Idling)
    FpsIdling fpsIdling;

    // --------------- DPI Handling -----------
    // Hello ImGui will try its best to automatically handle DPI scaling for you.
    // If it fails, look at DpiAwareParams (and the corresponding Ini file settings)
    DpiAwareParams dpiAwareParams;

    // --------------- Misc -------------------

    // `useImGuiTestEngine`: _bool, default=false_.
    // Set this to true if you intend to use Dear ImGui Test & Automation Engine
    //     ( https://github.com/ocornut/imgui_test_engine )
    // HelloImGui must be compiled with the option -DHELLOIMGUI_WITH_TEST_ENGINE=ON
    // See demo in src/hello_imgui_demos/hello_imgui_demo_test_engine.
    // License:
    // imgui_test_engine is subject to a specific license:
    //     https://github.com/ocornut/imgui_test_engine/blob/main/imgui_test_engine/LICENSE.txt)
    // (TL;DR: free for individuals, educational, open-source and small businesses uses.
    //  Paid for larger businesses.)
    bool useImGuiTestEngine = false;

    // `emscripten_fps`: _int, default = 0_.
    // Set the application refresh rate
    // (only used on emscripten: 0 stands for "let the app or the browser decide")
    int emscripten_fps = 0;

    #ifdef HELLOIMGUI_WITH_REMOTE_DISPLAY
    RemoteParams remoteParams; // Parameters for Remote display (experimental, unsupported)
    #endif
};
// @@md


// @@md#IniIniSettingsLocation

// IniSettingsLocation returns the path to the ini file for the application settings.
std::string IniSettingsLocation(const RunnerParams& runnerParams);

// HasIniSettings returns true if the ini file for the application settings exists.
bool HasIniSettings(const RunnerParams& runnerParams);

// DeleteIniSettings deletes the ini file for the application settings.
void DeleteIniSettings(const RunnerParams& runnerParams);

// @@md

// --------------------------------------------------------------------------------------------------------------------

// @@md#SimpleRunnerParams

// SimpleRunnerParams is a struct that contains simpler params adapted for simple use cases.
//For example, this is sufficient to run an application:
//    ```cpp
//    void MyGui() {
//        ImGui::Text("Hello, world");
//        if (ImGui::Button("Exit"))
//            HelloImGui::GetRunnerParams()->appShallExit = true;
//    }
//
//    int main(){
//        auto params = HelloImGui::SimpleRunnerParams {
//            .guiFunction = MyGui, .windowSizeAuto = true, .windowTitle = "Example"
//        };
//        HelloImGui::Run(params);
//    }
//    ```
struct SimpleRunnerParams
{
    // `guiFunction`: _VoidFunction_.
    //  Function that renders the Gui.
    VoidFunction guiFunction = EmptyVoidFunction();
    // `windowTitle`: _string, default=""_.
    //  Title of the application window
    std::string windowTitle = "";

    // `windowSizeAuto`: _bool, default=false_.
    //  If true, the size of the window will be computed from its widgets.
    bool windowSizeAuto = false;

    // `windowRestorePreviousGeometry`: _bool, default=true_.
    //  If true, restore the size and position of the window between runs.
    bool windowRestorePreviousGeometry = false;

    // `windowSize`: _ScreenSize, default={800, 600}_.
    //  Size of the window
    // The size will be handled as if it was specified for a 96PPI screen
    // (i.e. a given size will correspond to the same physical size on different screens, whatever their DPI)
    ScreenSize windowSize = DefaultWindowSize;

    // `fpsIdle`: _float, default=9_.
    //  FPS of the application when idle (set to 0 for full speed).
    float fpsIdle = 9.f;

    // `enableIdling`: _bool, default=true_.
    //  Disable idling at startup by setting this to false
    //  When running, use:
    //      HelloImGui::GetRunnerParams()->fpsIdling.enableIdling = false;
    bool  enableIdling = true;

    RunnerParams ToRunnerParams() const;
};
// @@md

}  // namespace HelloImGui