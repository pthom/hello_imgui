<span id="TOC"/></span>

* [API](#api)
  * [HelloImGui::Run()](#helloimguirun)
  * [Runner params](#runner-params)
      * [Diagram](#diagram)
      * [RunnerParams](#runnerparams)
        * [Simple runner params](#simple-runner-params)
        * [Full params](#full-params)
  * [Runner callbacks](#runner-callbacks)
      * [RunnerCallbacks](#runnercallbacks)
      * [MobileCallbacks](#mobilecallbacks)
  * [Application window params](#application-window-params)
    * [AppWindowParams](#appwindowparams)
    * [WindowGeometry](#windowgeometry)
  * [ImGui window params](#imgui-window-params)
      * [ImGuiWindowParams](#imguiwindowparams)
      * [Default window types](#default-window-types)
  * [Docking](#docking)
      * [Docking Params: Example usage](#docking-params-example-usage)
      * [Docking Splits](#docking-splits)
      * [Dockable window](#dockable-window)
      * [Docking Params](#docking-params)
  * [Applications assets](#applications-assets)
      * [Assets Files structure](#assets-files-structure)
      * [Load Assets as data buffer](#load-assets-as-data-buffer)
      * [Get assets path](#get-assets-path)
  * [Display images](#display-images)
      * [HelloImGui::ImageFromAsset](#helloimguiimagefromasset)
  * [Backend Pointers](#backend-pointers)

# API

## HelloImGui::Run()

See [hello_imgui.h](hello_imgui.h).


__HelloImGui::Run()__ will run an application with a single call.

Three signatures are provided:

* `HelloImGui::Run(RunnerParams &)`: full signature, the most customizable version.
   Runs an application whose params and Gui are provided by runnerParams.

* `HelloImGui::Run(const SimpleRunnerParams&)`:
   Runs an application, using simpler params.

* `HelloImGui::Run(guiFunction, windowTitle, windowSize, windowSizeAuto=false, restoreLastWindowGeometry=false, fpsIdle=10)`

__HelloImGui::GetRunnerParams()__ is a convenience function that will return the runnerParams of the current application.


## Runner params

See [runner_params.h](runner_params.h).

#### Diagram

_RunnerParams_ contains all the settings and callbacks in order to run an application. 
The diagram below summarize all the possible settings and callbacks (which are explained in detail later in this document).

![a](doc_src/hello_imgui_diagram.png)

#### RunnerParams

##### Simple runner params

**RunnerParams** is a struct that contains simpler params adapted for simple uses

 Members:
* `guiFunction`: _VoidFunction_.
   Function that renders the Gui.
* `windowTitle`: _string, default=""_.
   Title of the application window
* `windowSizeAuto`: _bool, default=false_.
   If true, the size of the window will be computed from its widgets.
* `windowRestorePreviousGeometry`: _bool, default=true_.
   If true, restore the size and position of the window between runs.
* `windowSize`: _ScreenSize, default={800, 600}_.
   Size of the window
* `fpsIdle`: _float, default=10_.
   FPS of the application when idle (set to 0 for full speed).

##### Full params

**RunnerParams** is a struct that contains all the settings and callbacks needed to run an application.

 Members:
* `callbacks`: _see [runner_callbacks.h](runner_callbacks.h)_.
    callbacks.ShowGui() will render the gui, ShowMenus() will show the menus, etc.
* `appWindowParams`: _see [app_window_params.h](app_window_params.h)_.
    application Window Params (position, size, title)
* `imGuiWindowParams`: _see [imgui_window_params.h](imgui_window_params.h)_.
    imgui window params (use docking, showMenuBar, ProvideFullScreenWindow, etc)
* `dockingParams`: _see [docking_params.h](docking_params.h)_.
    dockable windows content and layout
* `backendPointers`: _see [backend_pointers.h](backend_pointers.h)_.
   A struct that contains optional pointers to the backend implementations. These pointers will be filled
   when the application starts
* `backendType`: _enum BackendType, default=BackendType::FirstAvailable_
  Select the wanted backend type between `Sdl`, `Glfw` and `Qt`. Only useful when multiple backend are compiled
  and available.
* `appShallExit`: _bool, default=false_.
   Will be set to true by the app when exiting.
   _Note: 'appShallExit' has no effect on Mobile Devices (iOS, Android) and under emscripten, since these apps
   shall not exit._
* `fpsIdle`: _float, default=10_.
  ImGui applications can consume a lot of CPU, since they update the screen very frequently.
  In order to reduce the CPU usage, the FPS is reduced when no user interaction is detected.
  This is ok most of the time but if you are displaying animated widgets (for example a live video),
  you may want to ask for a faster refresh: either increase fpsIdle, or set it to 0 for maximum refresh speed
  (you can change this value during the execution depending on your application refresh needs)
* `emscripten_fps`: _int, default = 0_.
  Set the application refresh rate (only used on emscripten: 0 stands for "let the app or the browser decide")

----

## Runner callbacks

See [runner_callbacks.h](runner_callbacks.h).

#### RunnerCallbacks


 **RunnerCallbacks** is a struct that contains the callbacks that are called by the application

 _Members_

* `ShowGui`: *VoidFunction, default=empty*.
  Fill it with a function that will add your widgets.

* `ShowMenus`: *VoidFunction, default=empty*.
    A function that will render your menus. Fill it with a function that will add ImGui menus by calling:
    _ImGui::BeginMenu(...) / ImGui::MenuItem(...) / ImGui::EndMenu()_

    _Notes:_
    * you do not need to call _ImGui::BeginMenuBar_ and _ImGui::EndMenuBar_
    * Some default menus can be provided: see _ImGuiWindowParams_ options
      (_showMenuBar, showMenu_App_QuitAbout, showMenu_View_)

* `ShowStatus`: *VoidFunction, default=empty*.
  A function that will add items to the status bar. Use small items (ImGui::Text for example),
  since the height of the status is 30. Also, remember to call ImGui::SameLine() between items.

* `PostInit`: *VoidFunction, default=empty*.
    You can here add a function that will be called once after OpenGL and ImGui are inited

* `BeforeExit`: *VoidFunction, default=empty*.
    You can here add a function that will be called once before exiting (when OpenGL and ImGui are
    still inited)

* `AnyBackendEventCallback`: *AnyBackendCallback, default=empty*.
  Callbacks for events from a specific backend. _Only implemented for SDL, where the event
  will be of type 'SDL_Event *'_
  This callback should return true if the event was handled and shall not be processed further.

* `LoadAdditionalFonts`: *VoidFunction, default=_LoadDefaultFont_WithFontAwesome*.
   A function that is called when fonts are ready to be loaded.
   By default, _LoadDefaultFont_WithFontAwesome_ is called but you can copy-customize it.

* `SetupImGuiConfig`: *VoidFunction, default=_ImGuiDefaultSettings::SetupDefaultImGuiConfig*.
    If needed, change ImGui config via SetupImGuiConfig (enable docking, gamepad, etc)

* `SetupImGuiStyle`: *VoidFunction, default=_ImGuiDefaultSettings::SetupDefaultImGuiConfig*.
    If needed, setup your own style by providing your own SetupImGuiStyle callback


* `mobileCallbacks`: *_MobileCallbacks_*. Callbacks that are called by the application
    when running under "Android, iOS and WinRT".
Notes:
  * 'mobileCallbacks' is present only if the target device is a mobile device (iOS, Android).
     Use `#ifdef HELLOIMGUI_MOBILEDEVICE` to detect this.
  * These events are currently handled only with SDL backend.



**VoidFunctionPointer** can hold any void(void) function.
````cpp
using VoidFunction = std::function<void(void)>
````

**AnyEventCallback** can hold any bool(void *) function.
  It is designed to handle callbacks for a specific backend.
````cpp
using AnyEventCallback = std::function<bool(void * backendEvent)>
````


#### MobileCallbacks


**MobileCallbacks** is a struct that contains callbacks that are called by the application
 when running under "Android, iOS and WinRT".
 These events are specific to mobile and embedded devices that have different requirements
 than your usual desktop application. These events must be handled quickly,
 since often the OS needs an immediate response and will terminate your process shortly
 after sending the event if you do not handle them apprpriately.

 Note: on mobile devices, it is not possible to "Quit" an application, it can only be put on Pause.

 * `OnDestroy`: _VoidFunction, default=empty_. The application is being terminated by the OS.
 * `OnLowMemory`: _VoidFunction, default=empty_. The application is low on memory, free memory if possible.
 * `OnPause`: _VoidFunction, default=empty_. The application is about to enter the background.
 * `OnResume`: _VoidFunction, default=empty_. The application is has come to foreground and is now interactive.

 Note: 'OnPause' and 'OnResume' are called twice consecutively under iOS (before and after entering background
 or foreground).


----

## Application window params

See [app_window_params.h](app_window_params.h).

### AppWindowParams


__AppWindowParams__ is a struct that defines the application window display params.
See [doc_src/hello_imgui_diagram.png](https://raw.githubusercontent.com/pthom/hello_imgui/master/src/hello_imgui/doc_src/hello_imgui_diagram.png)
for details.

Members:
* `windowTitle`: _string, default=""_. Title of the application window
* `windowGeometry`: _WindowGeometry_
  Enables to precisely set the window geometry (position, monitor, size, full screen, fake full screen, etc.)
   _Note: on a mobile device, the application will always be full screen._
* `restorePreviousGeometry`: _bool, default=false_.
  If true, then save & restore windowGeometry from last run (the geometry will be written in imgui_app_window.ini)
* `borderless`: _bool, default = false_.
* `resizable`: _bool, default = false_.

Output Member:
* `outWindowDpiFactor`: _float, default = 1_.
   This value is filled by HelloImGui during the window initialisation. On Windows and Linux, it can be > 1
   on high resolution monitors (on MacOS, the scaling is handled by the system).
   When loading fonts, their size should be multiplied by this factor.

### WindowGeometry


__WindowGeometry__ is a struct that defines the window geometry.

Members:
* `size`: _int[2], default="{800, 600}"_. Size of the application window
  used if fullScreenMode==NoFullScreen and sizeAuto==false
* `sizeAuto`: _bool, default=false_
  If true, adapt the app window size to the presented widgets
* `fullScreenMode`: _FullScreenMode, default=NoFullScreen_.
   You can choose between several full screen modes:
   ````cpp
        NoFullScreen,
        FullScreen,                    // Full screen with specified resolution
        FullScreenDesktopResolution,   // Full screen with current desktop mode & resolution
        FullMonitorWorkArea            // Fake full screen, maximized window on the selected monitor
    ````
* `positionMode`: _WindowPositionMode, default = OsDefault_.
   You can choose between several window position modes:
   ````cpp
        OsDefault,
        MonitorCenter,
        FromCoords,
    ````
* `monitorIdx`: _int, default = 0_.
  used if positionMode==MonitorCenter or if fullScreenMode!=NoFullScreen
* `windowSizeState`: _WindowSizeState, default=Standard_
   You can choose between several window size states:
   ````cpp
        Standard,
        Minimized,
        Maximized
    ````

----

## ImGui window params

See [imgui_window_params.h](imgui_window_params.h).

#### ImGuiWindowParams


__ImGuiWindowParams__ is a struct that defines the ImGui inner windows params
These settings affect the imgui inner windows inside the application window.
In order to change the application window settings, change the _AppWindowsParams_

 Members:

  * `defaultImGuiWindowType`: _DefaultImGuiWindowType, default=ProvideFullScreenWindow_.
      By default, a full window is provided in the background. You can still
      add windows on top of it, since the Z-order of this background window is always behind

  * `backgroundColor`: _ImVec4, default=ImVec4(0.45f, 0.55f, 0.60f, 1.00f)_.
      This is the "clearColor", only visible is defaultImGuiWindowType is NoDefaultWindow.

  * `showMenuBar`: _bool, default=false_.
    Show Menu bar on top of imgui main window
    You can customize the menu via `RunnerCallbacks.ShowMenus()`

  * `showMenu_App`: _bool, default=true_.
    If menu bar is shown, include or not the default app menu (with Quit button)

  * `showMenu_View`: _bool, default=true_.
    If menu bar is shown, include or not the default _View_ menu, that enables to change the layout and
    set the docked windows and status bar visibility)

  * `showStatusBar`: _bool, default=false_.
    Flag that enable to show a Status bar at the bottom. You can customize the status bar
    via RunnerCallbacks.ShowStatus()

  * `showStatus_Fps`: _bool, default=true_. If set, display the FPS in the status bar.

  * `configWindowsMoveFromTitleBarOnly`: _bool, default=true_.
    Make windows only movable from the title bar

  * `enableViewports`: _bool, default=false_. Enable multiple viewports (i.e multiple native windows)
    If true, you can drag windows outside out the main window in order to put their content into new native windows.

  * `tweakedTheme`: _string, default="ImGuiColorsDark"_.
    Change the ImGui theme. Several themes are available, you can query the list by calling
    HelloImGui::AvailableThemes()

#### Default window types


__DefaultImGuiWindowType__ is an enum class that defines whether or not a full screen background window is provided.

 Values:
  * _ProvideFullScreenWindow_: a full window is provided in the background
  * _ProvideFullScreenDockSpace_: a full screen dockspace is provided in the background
  * _NoDefaultWindow_: No default window is provided (except for ImGui's default "debug" window)


----

## Docking

See [docking_params.h](docking_params.h).


HelloImGui facilitates the use of dockable windows (based on ImGui [docking branch](https://github.com/ocornut/imgui/tree/docking)).

You can easily specify the default layout of the dockable windows, as well as their GUI code.
HelloImGui will then provide a "View" menu with options in order to show/hide the dockable windows, and to restore the default layout

![demo docking](../../docs/images/docking.gif)

Source for this example: [src/hello_imgui_demos/hello_imgui_demodocking](../../src/hello_imgui_demos/hello_imgui_demodocking)

This is done via the `DockingParams` struct: its member `dockingSplits` specifies the layout,
and its member `dockableWindows` specifies the list of dockable windows, along with their default location,
and their code (given by lambdas). See doc below for more details.


#### Docking Params: Example usage


Docking params: Example usage

````cpp
HelloImGui::RunnerParams runnerParams;
runnerParams.imGuiWindowParams.defaultImGuiWindowType =
    HelloImGui::DefaultImGuiWindowType::ProvideFullScreenDockSpace;

runnerParams.dockingParams.dockingSplits =
{
    // First, add a bottom space whose height is 25% of the app height
    // This will split the preexisting default dockspace "MainDockSpace"
    // in two parts.
    { "MainDockSpace", "BottomSpace", ImGuiDir_Down, 0.25 },
    // Then, add a space to the left which occupies a column
    // whose width is 25% of the app height
    { "MainDockSpace", "LeftSpace", ImGuiDir_Left, 0.25 }
    // We now have three spaces: "MainDockSpace", "BottomSpace", and "LeftSpace"
};
runnerParams.dockingParams.dockableWindows =
{
    // A Window named "Main" will be placed in "MainDockSpace".
    // Its Gui is provided by the VoidFunction "MainGui"
    {"Main", "MainDockSpace", MainGui},
    // A Log  window named "Logs" will be placed in "BottomSpace".
    // Its Gui is provided by the VoidFunction "ShowLogs"
    {"Logs", "BottomSpace", ShowLogs},
    // A Command panel named "Commands" will be placed in "LeftSpace".
    // Its Gui is provided by the VoidFunction "ShowCommandsPanel"
    {"Commands", "LeftSpace", ShowCommandsPanel}
};

runnerParams.imGuiWindowParams.showMenuBar = true;
runnerParams.imGuiWindowParams.showStatusBar = true;

HelloImGui::Run(runnerParams);
````


#### Docking Splits


**DockingSplit** is a struct that defines the way the docking splits should be applied on the screen
in order to create new Dock Spaces. _DockingParams_ contains a _vector[DockingSplit]_,
in order to partition the screen at your will.

_Members:_

* `initialDock`: _DockSpaceName (aka string)_

    id of the space that should be split.
    At the start, there is only one Dock Space named "MainDockSpace".
    You should start by partitioning this space, in order to create a new dock space.

* `newDock`: _DockSpaceName (aka string)_. id of the new dock space that will be created
* `direction`: *ImGuiDir_ (enum with ImGuiDir_Down, ImGuiDir_Down, ImGuiDir_Left, ImGuiDir_Right)*.
Direction where this dock space should be created
* `ratio`: _float, default=0.25f_. Ratio of the initialDock size that should be used by the new dock space


#### Dockable window


**DockableWindow** is a struct that represents a window that can be docked.

_Members:_

* `label`: _string_. Title of the window.
* `dockSpaceName`: _DockSpaceName (aka string)_. Id of the dock space where this window
   should initialy be placed
* `GuiFunction`: _VoidFuntion_. Any function that will render this window's Gui.
* `isVisible`: _bool, default=true_. Flag that indicates whether this window is visible or not.
* `canBeClosed`: _bool, default=true_. Flag that indicates whether the user can close this window.
* `callBeginEnd`: _bool, default=true_. Flag that indicates whether ImGui::Begin and ImGui::End
   calls should be added automatically (with the given "label"). Set to false if you want to call
   ImGui::Begin/End yourself
* `includeInViewMenu`: _bool, default=true_. Flag that indicates whether this window should be mentioned
   in the view menu.
* `imGuiWindowFlags`: _ImGuiWindowFlags, default=0_. Window flags, see enum ImGuiWindowFlags_
* `windowSize`: _ImVec2, default=(0.f, 0.f) (i.e let the app decide)_. Window size (unused if docked)
* `windowSizeCondition`: _ImGuiCond, default=ImGuiCond_FirstUseEver_. When to apply the window size.
* `windowPos`: _ImVec2, default=(0.f, 0.f) (i.e let the app decide)_. Window position (unused if docked)
* `windowPosCondition`: _ImGuiCond, default=ImGuiCond_FirstUseEver_. When to apply the window position.
* `focusWindowAtNextFrame`: _bool, default = false_. If set to true this window will be focused at the next frame.


#### Docking Params


**DockingParams** contains all the settings concerning the docking,
 together _with the Gui functions for the docked windows_.

 _Members:_

* `dockingSplits`: _vector[DockingSplit]_.
  Defines the way docking splits should be applied on the screen in order to create new Dock Spaces
* `dockableWindows`: _vector[DockableWindow]_.
  List of the dockable windows, together with their Gui code
* `resetUserDockLayout`: _bool, default=true_.
  Reset user layout at application startup

 _Helpers:_

 * `DockableWindow * dockableWindowOfName(const std::string & name)`: returns a pointer to a dockable window
 * `void focusDockableWindow(const std::string& name)`: will focus a dockable window


----

## Applications assets

See [hello_imgui_assets.h](hello_imgui_assets.h).

#### Assets Files structure


Assets located beside the application CMakeLists are embedded automatically.

For example, you can have the following project structure:
````
my_app/
├── CMakeLists.txt        # Your app's CMakeLists
├── assets/               # Its assets: for mobile devices and emscripten
│   └── fonts/            # they are embedded automatically by hello_imgui_add_app.cmake
│       └── my_font.ttf
├── my_app.main.cpp       # Its source code
````

Then you can load the asset "fonts/my_font.ttf", on all platforms.


#### Load Assets as data buffer


* `AssetFileData LoadAssetFileData(const char *assetPath)` will load an entire asset file into memory.
 This works on all platforms, including android.
 ````cpp
    struct AssetFileData
    {
        void * data = nullptr;
        size_t dataSize = 0;
    };
 ````
* `FreeAssetFileData(AssetFileData * assetFileData)` will free the memory.

  Note about ImGui: "ImGui::GetIO().Fonts->AddFontFromMemoryTTF" takes ownership of the data
  and will free the memory for you.


#### Get assets path


`std::string assetFileFullPath(const std::string& assetRelativeFilename)` will return the path to assets.

This works under all platforms __except Android__.
For compatibility with Android and other platforms, prefer to use `LoadAssetFileData` whenever possible.

* Under iOS it will give a path in the app bundle (/private/XXX/....)
* Under emscripten, it will be stored in the virtual filesystem at "/"
* Under Android, assetFileFullPath is *not* implemented, and will throw an error:
  assets can be compressed under android, and you cannot use standard file operations!
  Use LoadAssetFileData instead


----

## Display images

#### HelloImGui::ImageFromAsset

See [image_from_asset.h](image_from_asset.h).


* `HelloImGui::ImageFromAsset(const char *assetPath, size, ...)`: will display a static image from the assets.
* `bool HelloImGui::ImageButtonFromAsset(const char *assetPath, size, ...)`: will display a button using an image from the assets.
* `ImTextureID HelloImGui::ImTextureIdFromAsset(const char *assetPath)`: will return a texture ID for an image loaded from the assets.

Images are loaded when first displayed, and then cached (they will be freed just before the application exits).

For example, given this files structure:
````
├── CMakeLists.txt
├── assets/
│   └── my_image.jpg
└── my_app.main.cpp
````

then, you can display "my_image.jpg", using:

````cpp
HelloImGui::ImageFromAsset("my_image.jpg");
````

*Note: HelloImGui::ImageFromAsset only works with OpenGL backends. It will throw an exception on other backends*


## Backend Pointers


**BackendPointers** is a struct that contains optional pointers to the backend implementations (for SDL and GLFW).

These pointers will be filled when the application starts, and you can use them to customize
your application behavior using the selected backend.

 Members:
* `glfwWindow`: _void *, default=nullptr_. Pointer to the main GLFW window (of type `GLFWwindow*`).
  Only filled if the backend is GLFW.
* `sdlWindow`: _void *, default=nullptr_. Pointer to the main SDL window (of type `SDL_Window*`).
  Only filled if the backend is SDL (or emscripten + sdl)
* `sdlGlContext`: _void *, default=nullptr_. Pointer to SDL's GlContext (of type `SDL_GLContext`).
  Only filled if the backend is SDL (or emscripten + sdl)


----
