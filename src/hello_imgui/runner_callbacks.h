#pragma once
#include <functional>

namespace HelloImGui
{
/**
@@md#VoidFunction_AnyEventCallback

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
@@md
**/
using VoidFunction = std::function<void(void)>;
using AnyEventCallback = std::function<bool(void * backendEvent)>;
VoidFunction AppendCallback(const VoidFunction& previousCallback, const VoidFunction& newCallback);


inline VoidFunction EmptyVoidFunction() { return {}; }
inline AnyEventCallback EmptyEventCallback() {return {}; }

}


#include "hello_imgui/imgui_default_settings.h"

namespace HelloImGui
{
/**
@@md#MobileCallbacks

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

@@md
 */
struct MobileCallbacks
{
    VoidFunction OnDestroy = EmptyVoidFunction();
    VoidFunction OnLowMemory = EmptyVoidFunction();
    VoidFunction OnPause = EmptyVoidFunction();
    VoidFunction OnResume = EmptyVoidFunction();
};

/**
 @@md#RunnerCallbacks

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

@@md
 */
struct RunnerCallbacks
{
    VoidFunction ShowGui = EmptyVoidFunction();
    VoidFunction ShowMenus = EmptyVoidFunction();
    VoidFunction ShowAppMenuItems = EmptyVoidFunction();
    VoidFunction ShowStatus = EmptyVoidFunction();
    VoidFunction PostInit = EmptyVoidFunction();
    VoidFunction BeforeExit = EmptyVoidFunction();
    VoidFunction PreNewFrame = EmptyVoidFunction();

    AnyEventCallback AnyBackendEventCallback = EmptyEventCallback();

    VoidFunction LoadAdditionalFonts = (VoidFunction)(ImGuiDefaultSettings::LoadDefaultFont_WithFontAwesomeIcons);
    VoidFunction SetupImGuiConfig = (VoidFunction)(ImGuiDefaultSettings::SetupDefaultImGuiConfig);
    VoidFunction SetupImGuiStyle = (VoidFunction)(ImGuiDefaultSettings::SetupDefaultImGuiStyle);

#ifdef HELLOIMGUI_MOBILEDEVICE
    MobileCallbacks mobileCallbacks;
#endif
};

}  // namespace HelloImGui
