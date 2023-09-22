<span id="TOC"/></span>

* [API](#api)
  * [HelloImGui::Run()](#helloimguirun)
  * [Runner params](#runner-params)
      * [Diagram](#diagram)
      * [RunnerParams](#runnerparams)
        * [Simple runner params](#simple-runner-params)
        * [Full params](#full-params)
      * [Fps Idling](#fps-idling)
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
  * [Handling screen with high DPI: items positioning and font loading](#handling-screen-with-high-dpi-items-positioning-and-font-loading)
  * [Applications assets](#applications-assets)
      * [Assets Files structure](#assets-files-structure)
      * [Load Assets as data buffer](#load-assets-as-data-buffer)
      * [Get assets path](#get-assets-path)
  * [Display images](#display-images)
      * [HelloImGui::ImageFromAsset](#helloimguiimagefromasset)
  * [Backend Pointers](#backend-pointers)
  * [Store user settings in the ini file](#store-user-settings-in-the-ini-file)
  * [Switch between several layouts](#switch-between-several-layouts)

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


__Other utilities:__

* `HelloImGui::GetRunnerParams()`:
  a convenience function that will return the runnerParams of the current application

* `FrameRate(durationForMean = 0.5)`: Returns the current FrameRate.
  May differ from ImGui::GetIO().FrameRate, since one can choose the duration for the calculation of the mean value of the fps

## Runner params

See [runner_params.h](runner_params.h).

#### Diagram

_RunnerParams_ contains all the settings and callbacks in order to run an application. 
The diagram below summarize all the possible settings and callbacks (which are explained in detail later in this document).

![a](doc_src/hello_imgui_diagram.png)

#### RunnerParams

##### Simple runner params

**SimpleRunnerParams** is a struct that contains simpler params adapted for simple use cases.

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
* `fpsIdle`: _float, default=9_.
   FPS of the application when idle (set to 0 for full speed).

For example, this is sufficient to run an application:

```cpp
void MyGui() {
    ImGui::Text("Hello, world");
    if (ImGui::Button("Exit"))
        HelloImGui::GetRunnerParams()->appShallExit = true;
}

int main(){
    auto params = HelloImGui::SimpleRunnerParams {.guiFunction = MyGui, .windowSizeAuto = true, .windowTitle = "Example"};
    HelloImGui::Run(params);
}
```


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
* `alternativeDockingLayouts`: _vector<DockingParams>, default=empty_
   List of possible additional layout for the applications. Only used in advanced cases when several layouts are available.
* `rememberSelectedAlternativeLayout`: _bool, default=true_
   Shall the application remember the last selected layout. Only used in advanced cases when several layouts are available.
* `backendPointers`: _see [backend_pointers.h](backend_pointers.h)_.
   A struct that contains optional pointers to the backend implementations. These pointers will be filled
   when the application starts
* `backendType`: _enum BackendType, default=BackendType::FirstAvailable_
  Select the wanted backend type between `Sdl`, `Glfw` and `Qt`. Only useful when multiple backend are compiled
  and available.
* `fpsIdling`: _FpsIdling_. Idling parameters (set fpsIdling.enableIdling to false to disable Idling)
* `iniFilename`: _string, default = ""_
  Sets the ini filename under which imgui will save its params. Path is relative to the current app working dir.
  If empty, then the ini file name will be derived from appWindowParams.windowTitle (if both are empty, the ini filename will be imgui.ini).
  Note: if appWindowParams.restorePreviousGeometry is true, then HelloImGui will also store the app window size and position into "iniFilename + _appWindow.ini"
* `iniFilename_useAppWindowTitle`: _bool, default = true_.
  Shall the iniFilename be derived from appWindowParams.windowTitle if empty
* `appShallExit`: _bool, default=false_.
   During execution, set this to true to exit the app.
   _Note: 'appShallExit' has no effect on Mobile Devices (iOS, Android) and under emscripten, since these apps
   shall not exit._
* `emscripten_fps`: _int, default = 0_.
  Set the application refresh rate (only used on emscripten: 0 stands for "let the app or the browser decide")

#### Fps Idling

**FpsIdling** is a struct that contains Fps Idling parameters

* `fpsIdle`: _float, default=9_.
  ImGui applications can consume a lot of CPU, since they update the screen very frequently.
  In order to reduce the CPU usage, the FPS is reduced when no user interaction is detected.
  This is ok most of the time but if you are displaying animated widgets (for example a live video),
  you may want to ask for a faster refresh: either increase fpsIdle, or set it to 0 for maximum refresh speed
  (you can change this value during the execution depending on your application refresh needs)
* `enableIdling`: _bool, default=true_.
  Set this to false to disable idling (this can be changed dynamically during execution)
* `isIdling`: bool (dynamically updated during execution)
  This bool will be updated during the application execution, and will be set to true when it is idling.
* `rememberEnableIdling`: _bool, default=true_.
  If true, the last value of enableIdling is restored from the settings at startup.

See [runner_params.h](runner_params.h).


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

* `ShowAppMenuItems`: *VoidFunction, default=empty*.
  A function that will render items that will be placed in the App menu.
  They will be placed before the "Quit" MenuItem, which is added automatically by HelloImGui.
  This will be displayed only if ImGuiWindowParams.showMenu_App is true

* `ShowStatus`: *VoidFunction, default=empty*.
  A function that will add items to the status bar. Use small items (ImGui::Text for example),
  since the height of the status is 30. Also, remember to call ImGui::SameLine() between items.

* `PostInit`: *VoidFunction, default=empty*.
    You can here add a function that will be called once after OpenGL and ImGui are inited, but before
    the backend callback are initialized.
    If you, for instance, want to add your own glfw callbacks, you should use this function to do so."

* `BeforeExit`: *VoidFunction, default=empty*.
    You can here add a function that will be called once before exiting (when OpenGL and ImGui are
    still inited)

 * `PreNewFrame`: *VoidFunction, default=empty*.
    You can here add a function that will be called at each frame, and before the call to ImGui::NewFrame().
    It is a good place to dynamically add new fonts, or dynamically add new dockable windows.

* `AnyBackendEventCallback`: *AnyBackendCallback, default=empty*.
  Callbacks for events from a specific backend. _Only implemented for SDL, where the event
  will be of type 'SDL_Event *'_
  This callback should return true if the event was handled and shall not be processed further.
  Note: in the case of GLFW, you should use register them in `PostInit`

* `LoadAdditionalFonts`: *VoidFunction, default=_LoadDefaultFont_WithFontAwesome*.
   A function that is called once, when fonts are ready to be loaded.
   By default, _LoadDefaultFont_WithFontAwesome_ is called but you can copy and customize it.
   (LoadDefaultFont_WithFontAwesome will load from assets/fonts/ but reverts to the ImGui embedded font if not found)

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
```cpp
using VoidFunction = std::function<void(void)>
```

**AnyEventCallback** can hold any bool(void *) function.
  It is designed to handle callbacks for a specific backend.
```cpp
using AnyEventCallback = std::function<bool(void * backendEvent)>
```

**AppendCallback** can compose two callbacks. Use this when you want to set a callback and keep the (maybe) preexisting one.

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
* `borderless`: _bool, default = false_. Should the window have borders. This is taken into account at
creation.
* `resizable`: _bool, default = false_. Should the window have borders. This is taken into account at
creation.
* `hidden`: _bool, default = false_. Should the window be hidden. This is taken into account dynamically (you
can show/hide the window with this). Full screen windows cannot be hidden.@@md
**/
struct AppWindowParams
{
    std::string windowTitle;

    WindowGeometry windowGeometry;

    // if true, then save & restore from last run
    bool restorePreviousGeometry = false;

    bool borderless = false;
    bool resizable = true;
    bool hidden = false;
};

}  // namespace HelloImGui
### WindowGeometry


__WindowGeometry__ is a struct that defines the window geometry.

Members:
* `size`: _int[2], default="{800, 600}"_. Size of the application window
  used if fullScreenMode==NoFullScreen and sizeAuto==false
* `sizeAuto`: _bool, default=false_
  If true, adapt the app window size to the presented widgets.
  After the first frame was displayed, HelloImGui will measure its size,
  and the backend application window will be resized. As a consequence, the application window size may
  vary between the first and the second frame.

* `fullScreenMode`: _FullScreenMode, default=NoFullScreen_.
   You can choose between several full screen modes:
   ```cpp
        NoFullScreen,
        FullScreen,                    // Full screen with specified resolution
        FullScreenDesktopResolution,   // Full screen with current desktop mode & resolution
        FullMonitorWorkArea            // Fake full screen, maximized window on the selected monitor
    ```
* `positionMode`: _WindowPositionMode, default = OsDefault_.
   You can choose between several window position modes:
   ```cpp
        OsDefault,
        MonitorCenter,
        FromCoords,
    ```
* `monitorIdx`: _int, default = 0_.
  used if positionMode==MonitorCenter or if fullScreenMode!=NoFullScreen
* `windowSizeState`: _WindowSizeState, default=Standard_
   You can choose between several window size states:
   ```cpp
        Standard,
        Minimized,
        Maximized
    ```
* `windowSizeMeasureMode`: _WindowSizeMeasureMode_, default=RelativeTo96Ppi
  how the window size is specified:
  * RelativeTo96Ppi enables to give screen size that are independant from the screen density.
     For example, a window size expressed as 800x600 will correspond to a size
        - 800x600 (in screen coords) if the monitor dpi is 96
        - 1600x120 (in screen coords) if the monitor dpi is 192
      (this works with Glfw. With SDL, it only works under windows)
  * ScreenCoords: measure window size in screen coords
    (Note: screen coordinates might differ from real pixels on high dpi screen)

* `resizeAppWindowAtNextFrame`: _bool_, default=false;
  If you set this to flag to true at any point during the execution, the application window
  will then try to resize based on its content on the next displayed frame,
  and this flag will subsequently be set to false.
  Example:
  ```cpp
  // Will resize the app window at next displayed frame
  HelloImGui::GetRunnerParams()->appWindowParams.windowGeometry.resizeAppWindowAtNextFrame = true;
  ```

  :::Note: this flag is intended to be used during execution, not at startup (use sizeAuto at startup):::

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
    If menu bar is shown, include or not the default app menu

   * `showMenu_App_Quit`: _bool, default=true_.
    Include or not a "Quit" item in the default app menu.
    Set this to false if you intend to provide your own quit callback with possible user confirmation
    (and implement it inside RunnerCallbacks.ShowAppMenuItems)

  * `showMenu_View`: _bool, default=true_.
    If menu bar is shown, include or not the default _View_ menu, that enables to change the layout and
    set the docked windows and status bar visibility)

  * `showStatusBar`: _bool, default=false_.
    Flag that enable to show a Status bar at the bottom. You can customize the status bar
    via RunnerCallbacks.ShowStatus()
  * `showStatus_Fps`: _bool, default=true_. If set, display the FPS in the status bar.
  * `rememberStatusBarSettings`: _bool, default=true_. If set, showStatusBar and showStatus_Fps are stored in the application settings.

  * `configWindowsMoveFromTitleBarOnly`: _bool, default=true_.
    Make windows only movable from the title bar

  * `enableViewports`: _bool, default=false_. Enable multiple viewports (i.e multiple native windows)
    If true, you can drag windows outside out the main window in order to put their content into new native windows.

   * `menuAppTitle`: _string, default=""_. Set the title of the App menu. If empty, this menu name will use
     the "windowTitle" from AppWindowParams

  * `tweakedTheme`: _ImGuiTheme::ImGuiTweakedTheme_.
    Change the ImGui theme. Several themes are available, you can query the list by calling
    HelloImGui::AvailableThemes()
  * `showMenu_View_Themes`: _bool, default=true_.
    Show theme selection in view menu
  * `rememberTheme`: _bool, default=true_.
    Remember selected theme

#### Default window types


__DefaultImGuiWindowType__ is an enum class that defines whether or not a full screen background window is provided.

 Values:
  * _ProvideFullScreenWindow_: a full window is provided in the background
  * _ProvideFullScreenDockSpace_: a full screen dockspace is provided in the background
  * _NoDefaultWindow_: No default window is provided (except for ImGui's default "debug" window)


----

## Docking

See [docking_params.h](docking_params.h).


HelloImGui makes it easy to use dockable windows (based on ImGui [docking branch](https://github.com/ocornut/imgui/tree/docking)).

You can define several layouts and switch between them:  each layout which will remember the user modifications and the list of opened windows

HelloImGui will then provide a "View" menu with options to show/hide the dockable windows, restore the default layout, switch between layouts, etc.

![demo docking](https://traineq.org/ImGuiBundle/HelloImGuiLayout.gif)

* Source for this example: [src/hello_imgui_demos/hello_imgui_demodocking](../../src/hello_imgui_demos/hello_imgui_demodocking)
* [Video explanation on YouTube](https://www.youtube.com/watch?v=XKxmz__F4ow) (5 minutes)


The different available layouts are provided inside RunnerParams via the two members below:
```cpp
struct RunnerParams
{
    ...
    DockingParams dockingParams;                            // default layout of the application
    std::vector<DockingParams> alternativeDockingLayouts;   // optional alternative layouts
    ...
};
```

And `DockingParams` contains members that define a layout:

```cpp
struct DockingParams
{
    std::string layoutName = "Default";          // displayed name of the layout
    std::vector<DockingSplit> dockingSplits;     // list of splits (which define spaces where the windows will be placed)
    std::vector<DockableWindow> dockableWindows; // list of windows (with their gui code, and specifying in which space they will be placed)
    ...
};
```

Inside DockingParams, the member `dockingSplits` specifies the layout, and the member `dockableWindows`
 specifies the list of dockable windows, along with their default location, and their code (given by lambdas).


#### Docking Params: Example usage


Below is an example that shows how to instantiate a layout:

1. First, define the docking splits:

```cpp
std::vector<HelloImGui::DockingSplit> CreateDefaultDockingSplits()
{
    //     Here, we want to split "MainDockSpace" (which is provided automatically) into three zones, like this:
    //    ___________________________________________
    //    |        |                                |
    //    | Command|                                |
    //    | Space  |    MainDockSpace               |
    //    |        |                                |
    //    |        |                                |
    //    |        |                                |
    //    -------------------------------------------
    //    |     MiscSpace                           |
    //    -------------------------------------------
    //

    // add a space named "MiscSpace" whose height is 25% of the app height.
    // This will split the preexisting default dockspace "MainDockSpace" in two parts.
    HelloImGui::DockingSplit splitMainMisc;
    splitMainMisc.initialDock = "MainDockSpace";
    splitMainMisc.newDock = "MiscSpace";
    splitMainMisc.direction = ImGuiDir_Down;
    splitMainMisc.ratio = 0.25f;

    // Then, add a space to the left which occupies a column whose width is 25% of the app width
    HelloImGui::DockingSplit splitMainCommand;
    splitMainCommand.initialDock = "MainDockSpace";
    splitMainCommand.newDock = "CommandSpace";
    splitMainCommand.direction = ImGuiDir_Left;
    splitMainCommand.ratio = 0.25f;

    std::vector<HelloImGui::DockingSplit> splits {splitMainMisc, splitMainCommand};
    return splits;
}
```

2. Then, define the dockable windows:

```cpp
std::vector<HelloImGui::DockableWindow> CreateDockableWindows(AppState& appState)
{
    // A Command panel named "Commands" will be placed in "CommandSpace". Its Gui is provided calls "CommandGui"
    HelloImGui::DockableWindow commandsWindow;
    commandsWindow.label = "Commands";
    commandsWindow.dockSpaceName = "CommandSpace";
    commandsWindow.GuiFunction = [&] { CommandGui(appState); };

    // A Log window named "Logs" will be placed in "MiscSpace". It uses the HelloImGui logger gui
    HelloImGui::DockableWindow logsWindow;
    logsWindow.label = "Logs";
    logsWindow.dockSpaceName = "MiscSpace";
    logsWindow.GuiFunction = [] { HelloImGui::LogGui(); };

    ...
}
```

3. Finally, fill the RunnerParams

```cpp
HelloImGui::RunnerParams runnerParams;
runnerParams.imGuiWindowParams.defaultImGuiWindowType =
    HelloImGui::DefaultImGuiWindowType::ProvideFullScreenDockSpace;

runnerParams.dockingParams.dockingSplits = CreateDefaultDockingSplits();
runnerParams.dockingParams.dockableWindows = CreateDockableWindows();


HelloImGui::Run(runnerParams);
```


#### Docking Splits


**DockingSplit** is a struct that defines the way the docking splits should be applied on the screen
in order to create new Dock Spaces. _DockingParams_ contains a _vector[DockingSplit]_,
in order to partition the screen at your will.

_Members:_

* `initialDock`: _DockSpaceName (aka string)_

    id of the space that should be split.
    At the start, there is only one Dock Space named "MainDockSpace".
    You should start by partitioning this space, in order to create a new dock space.

* `newDock`: _DockSpaceName (aka string)_. id of the new dock space that will be created.
* `direction`: *ImGuiDir_ (enum with ImGuiDir_Down, ImGuiDir_Down, ImGuiDir_Left, ImGuiDir_Right)*.
Direction where this dock space should be created.
* `ratio`: _float, default=0.25f_. Ratio of the initialDock size that should be used by the new dock space.
* `nodeFlags`: *ImGuiDockNodeFlags_ (enum)*. Flags to apply to the new dock space (enable/disable resizing, splitting, tab bar, etc.)


#### Dockable window


**DockableWindow** is a struct that represents a window that can be docked.

_Members:_

* `label`: _string_. Title of the window.
* `dockSpaceName`: _DockSpaceName (aka string)_. Id of the dock space where this window
   should initially be placed
* `GuiFunction`: _VoidFunction_. Any function that will render this window's Gui.
* `isVisible`: _bool, default=true_. Flag that indicates whether this window is visible or not.
* `rememberIsVisible`: _bool, default=true_. Flag that indicates whether the window visibility should be saved in settings or not.
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
* `layoutName`: _string, default="default"_.
  Displayed name of the layout. Only used in advanced cases when several layouts are available.
* `layoutCondition`: _enum DockingLayoutCondition, default=DockingLayoutCondition::FirstUseEver_.
  When to apply the docking layout. Choose between FirstUseEver (apply once, then keep user preference),
  ApplicationStart (always reapply at application start), and Never.
* `mainDockSpaceNodeFlags`: _ImGuiDockNodeFlags (enum), default=ImGuiDockNodeFlags_PassthruCentralNode_
   Flags to apply to the main dock space (enable/disable resizing, splitting, tab bar, etc.).
   Most flags are inherited by children dock spaces. You can also set flags for specific dock spaces via `DockingSplit.nodeFlags`
* `layoutReset`: _bool, default=false_.
  Reset layout on next frame, i.e. drop the layout customizations which were applied manually by the user.
  (layoutReset will be set to false after applying)

 _Helpers:_

 * `DockableWindow * dockableWindowOfName(const std::string & name)`: returns a pointer to a dockable window
 * `bool focusDockableWindow(const std::string& name)`: will focus a dockable window (and make its tab visible if needed)
 * `optional<ImGuiID> dockSpaceIdFromName(const std::string& dockSpaceName)`: may return the ImGuiID corresponding
   to the dockspace with this name.
   **Warning**: this will work reliably only if layoutCondition = DockingLayoutCondition::ApplicationStart. In other
   cases, the ID may be cached by ImGui himself at the first run, and HelloImGui will *not* know it on subsequent runs!

---

## Handling screen with high DPI: items positioning and font loading


Special care must be taken in order to correctly handle screen with high DPI (for example, almost all recent laptops screens).
Otherwise, widgets might be misplaced or too small, and font rendering might be blurry or too small.

### How to position widgets on a window in a Dpi independent way

Using ImVec2 with fixed values is *almost always a bad idea* if you intend your application to be used on high DPI screens.
Instead you can:
* either multiply those values by ImGui::GetFontSize()
* or use `HelloImGui::EmToVec2(x, y)` which will do this multiplication for you. Em stand for the `em` measurements,
   as used in CSS: 1em simply correspond to the current font height.


### How to load fonts for a crisp font rendering and a correct size

HelloImGui provides `HelloImGui::DpiFontLoadingFactor()` which corresponds to:
    `DpiWindowSizeFactor() * 1.f / ImGui::GetIO().FontGlobalScale`
              where DpiWindowSizeFactor() is equal to `CurrentScreenPixelPerInch / 96` under windows and linux, 1 under macOS

==> When loading fonts, multiply their size by this factor!

### More details on DPI handling with different OS and backends

Let's consider screen whose physical pixel resolution is 3600x2000, but which will displayed with a scaling factor of 200%,
 so that widgets do not look too small on it.

The way it is handled depends on the OS:
- On MacOS, the screen will be seen as having a resolution of 1800x1000, and the OS handles the resizing by itself.
- On Linux, and on Windows if the application is DPI aware, the screen will be seen as having a resolution of 3600x2000.
- On Windows if the application is not DPI aware, the screen will be seen as having a resolution of 1800x1000

By default, if using the glfw backend, applications will be Dpi aware under windows.
Sdl applications are normally not Dpi aware. However HelloImGui makes them Dpi aware when using the sdl backend.


### HelloImGui Dpi aware C++ API

`HelloImGui::EmSize()` (C++) and `hello_imgui.em_size()` (Python) return the visible font size on the screen.
For reproducible results, even on HighDPI screens, always scale your widgets and windows relatively to this size.
 It is somewhat comparable to the [em CSS Unit](https://lyty.dev/css/css-unit.html).

`HelloImGui::EmToVec2(x, y)` (C++) and `hello_imgui.em_to_vec2(x,y)` (Python) return an ImVec2 that you can use
 to size or place your widgets in a DPI independent way.

`HelloImGui::EmSize(nbLines)` (C++) and `hello_imgui.em_size(nb_lines)` (Python) return a size corresponding to nbLines text lines

`HelloImGui::DpiFontLoadingFactor()` (C++) and `hello_imgui.dpi_font_loading_factor()` (Python) return a factor by
 which you shall multiply your font sizes when loading fonts manually with _ImGui::GetIO().Fonts->AddFont..._
 HelloImGui::LoadFontTTF does this by default.

`HelloImGui::ImGuiDefaultFontGlobalScale()` (C++) and `hello_imgui.imgui_default_font_global_scale()` (Python) returns the
 default value that should be stored inside `ImGui::GetIO().FontGlobalScale`.
 Under windows and linux, this is always 1: no rescaling should be done by ImGui. Under macOS and emscripten,
 this can be < 1 (for example it will be 0.5 if the dpi scaling is 200%)

----

## Applications assets

See [hello_imgui_assets.h](hello_imgui_assets.h).

#### Assets Files structure


Assets located beside the application CMakeLists are embedded automatically.

For example, you can have the following project structure:
```
my_app/
├── CMakeLists.txt        # Your app's CMakeLists
├── assets/               # Its assets: for mobile devices and emscripten
│         └── fonts/            # they are embedded automatically by hello_imgui_add_app.cmake
│             └── my_font.ttf
├── my_app.main.cpp       # Its source code
```

Then you can load the asset "fonts/my_font.ttf", on all platforms.


#### Load Assets as data buffer


* `AssetFileData LoadAssetFileData(const char *assetPath)` will load an entire asset file into memory.
 This works on all platforms, including android.
 ```cpp
    struct AssetFileData
    {
        void * data = nullptr;
        size_t dataSize = 0;
    };
 ```
* `FreeAssetFileData(AssetFileData * assetFileData)` will free the memory.

  Note about ImGui: "ImGui::GetIO().Fonts->AddFontFromMemoryTTF" takes ownership of the data
  and will free the memory for you.


#### Get assets path


`std::string AssetFileFullPath(const std::string& assetRelativeFilename)` will return the path to assets.

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
```
├── CMakeLists.txt
├── assets/
│   └── my_image.jpg
└── my_app.main.cpp
```

then, you can display "my_image.jpg", using:

```cpp
HelloImGui::ImageFromAsset("my_image.jpg");
```

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


## Store user settings in the ini file

See [hello_imgui.h](hello_imgui.h).


You may store additional user settings in the application settings. This is provided as a convenience only,
and it is not intended to store large quantities of text data. Use sparingly.

* `SaveUserPref(string userPrefName, string userPrefContent)`:
  Shall be called in the callback runnerParams.callbacks.BeforeExit

* `string LoadUserPref(string& userPrefName)`
  Shall be called in the callback runnerParams.callbacks.PostInit

## Switch between several layouts

See [hello_imgui.h](hello_imgui.h).


 In advanced cases when several layouts are available, you can switch between layouts.
(see demo inside [hello_imgui_demodocking.main.cpp](../hello_imgui_demos/hello_imgui_demodocking/hello_imgui_demodocking.main.cpp))

* `SwitchLayout(layoutName)`
  Changes the application current layout. Only used in advanced cases when several layouts are available,
  i.e. if you filled runnerParams.alternativeDockingLayouts.
* `CurrentLayoutName()`: returns the name of the current layout


----
