#pragma once
#include "hello_imgui/imgui_default_settings.h"
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


// @@md#MobileCallbacks
//
// MobileCallbacks is a struct that contains callbacks that are called by the application
// when running under "Android, iOS and WinRT".
// These events are specific to mobile and embedded devices that have different
// requirements from your usual desktop application.
// These events must be handled quickly, since often the OS needs an immediate response
// and will terminate your process shortly after sending the event
// if you do not handle them appropriately.
// On mobile devices, it is not possible to "Quit" an application,
// it can only be put on Pause.
struct MobileCallbacks
{
    //`OnDestroy`: The application is being terminated by the OS.
    VoidFunction OnDestroy = EmptyVoidFunction();

    //`OnLowMemory`: _VoidFunction, default=empty_.
    // When the application is low on memory, free memory if possible.
    VoidFunction OnLowMemory = EmptyVoidFunction();

    //`OnPause`: The application is about to enter the background.
    VoidFunction OnPause = EmptyVoidFunction();

    //`OnResume`: The application came to foreground and is now interactive.
    // Note: 'OnPause' and 'OnResume' are called twice consecutively under iOS
    // (before and after entering background or foreground).
    VoidFunction OnResume = EmptyVoidFunction();
};
// @@md

// @@md#RunnerCallbacks
//
// RunnerCallbacks is a struct that contains the callbacks
// that are called by the application
//
struct RunnerCallbacks
{
    // --------------- GUI Callbacks -------------------

    // `ShowGui`: Fill it with a function that will add your widgets.
    VoidFunction ShowGui = EmptyVoidFunction();

    // `ShowMenus`: Fill it with a function that will add ImGui menus by calling:
    //       ImGui::BeginMenu(...) / ImGui::MenuItem(...) / ImGui::EndMenu()
    //   Notes:
    //   * you do not need to call ImGui::BeginMenuBar and ImGui::EndMenuBar
    //   * Some default menus can be provided:
    //     see ImGuiWindowParams options:
    //         _showMenuBar, showMenu_App_QuitAbout, showMenu_View_
    VoidFunction ShowMenus = EmptyVoidFunction();

    // `ShowAppMenuItems`: A function that will render items that will be placed
    // in the App menu. They will be placed before the "Quit" MenuItem,
    // which is added automatically by HelloImGui.
    //  This will be displayed only if ImGuiWindowParams.showMenu_App is true
    VoidFunction ShowAppMenuItems = EmptyVoidFunction();

    // `ShowStatus`: A function that will add items to the status bar.
    //  Use small items (ImGui::Text for example), since the height of the status is 30.
    //  Also, remember to call ImGui::SameLine() between items.
    VoidFunction ShowStatus = EmptyVoidFunction();


    // --------------- Startup sequence callbacks -------------------

    // `PostInit`: You can here add a function that will be called once after OpenGL
    //  and ImGui are inited, but before the backend callback are initialized.
    //  If you, for instance, want to add your own glfw callbacks,
    //  you should use this function to do so.
    VoidFunction PostInit = EmptyVoidFunction();

    // `LoadAdditionalFonts`: default=_LoadDefaultFont_WithFontAwesome*.
    //  A function that is called once, when fonts are ready to be loaded.
    //  By default, _LoadDefaultFont_WithFontAwesome_ is called,
    //  but you can copy and customize it.
    //  (LoadDefaultFont_WithFontAwesome will load fonts from assets/fonts/
    //  but reverts to the ImGui embedded font if not found)
    VoidFunction LoadAdditionalFonts =
        (VoidFunction)(ImGuiDefaultSettings::LoadDefaultFont_WithFontAwesomeIcons);

    // `SetupImGuiConfig`: default=_ImGuiDefaultSettings::SetupDefaultImGuiConfig*.
    //  If needed, change ImGui config via SetupImGuiConfig
    //  (enable docking, gamepad, etc)
    VoidFunction SetupImGuiConfig =
        (VoidFunction)(ImGuiDefaultSettings::SetupDefaultImGuiConfig);

    // `SetupImGuiStyle`: default=_ImGuiDefaultSettings::SetupDefaultImGuiConfig*.
    //  If needed, set your own style by providing your own SetupImGuiStyle callback
    VoidFunction SetupImGuiStyle =
        (VoidFunction)(ImGuiDefaultSettings::SetupDefaultImGuiStyle);

    // `RegisterTests`: A function that is called once ImGuiTestEngine is ready
    // to be filled with tests and automations definitions.
    VoidFunction RegisterTests = EmptyVoidFunction();


    // --------------- Exit sequence callbacks -------------------

    // `BeforeExit`: You can here add a function that will be called once before exiting
    //  (when OpenGL and ImGui are still inited)
    VoidFunction BeforeExit = EmptyVoidFunction();

    // `BeforeExit_PostCleanup`: You can here add a function that will be called once
    // before exiting (after OpenGL and ImGui have been stopped)
    VoidFunction BeforeExit_PostCleanup = EmptyVoidFunction();


    // --------------- Callbacks in the render loop -------------------

    // `PreNewFrame`: You can here add a function that will be called at each frame,
    //  and before the call to ImGui::NewFrame().
    //  It is a good place to dynamically add new fonts, or new dockable windows.
    VoidFunction PreNewFrame = EmptyVoidFunction();

    // `BeforeImGuiRender`: You can here add a function that will be called at each frame,
    //  after the user Gui code, and just before the call to
    //  ImGui::Render() (which will also call ImGui::EndFrame()).
    VoidFunction BeforeImGuiRender = EmptyVoidFunction();

    // `AfterSwap`: You can here add a function that will be called at each frame,
    //  after the Gui was rendered and swapped to the screen.
    VoidFunction AfterSwap = EmptyVoidFunction();

    // `CustomBackground`:
    //  By default, the background is cleared using ImGuiWindowParams.backgroundColor.
    //  If set, this function gives you full control over the background that is drawn
    //  behind the Gui. An example use case is if you have a 3D application
    //  like a mesh editor, or game, and just want the Gui to be drawn
    //  on top of that content.
    VoidFunction CustomBackground = EmptyVoidFunction();

    // `AnyBackendEventCallback`:
    //  Callbacks for events from a specific backend. _Only implemented for SDL.
    //  where the event will be of type 'SDL_Event *'_
    //  This callback should return true if the event was handled
    //  and shall not be processed further.
    //  Note: in the case of GLFW, you should use register them in `PostInit`
    AnyEventCallback AnyBackendEventCallback = EmptyEventCallback();


    // --------------- Mobile callbacks -------------------
#ifdef HELLOIMGUI_MOBILEDEVICE
    // `mobileCallbacks`: Callbacks that are called by the application
    //  when running under "Android, iOS and WinRT".
    // Notes:
    //  * 'mobileCallbacks' is present only if the target device
    //    is a mobile device (iOS, Android).
    //    Use `#ifdef HELLOIMGUI_MOBILEDEVICE` to detect this.
    //  * These events are currently handled only with SDL backend.
    MobileCallbacks mobileCallbacks;
#endif
};
// @@md

}  // namespace HelloImGui
