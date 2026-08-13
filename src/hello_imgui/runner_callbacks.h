#pragma once
#include "hello_imgui/imgui_default_settings.h"
#include "imgui.h"
#include <functional>
#include <map>
#include <string>
#include <vector>


namespace HelloImGui
{
// --------------------------------------------------------------------------------------------------------------------

// @@md#VoidFunction_AnyEventCallback

// VoidFunctionPointer can hold any void(void) function.
using VoidFunction = std::function<void(void)>;
inline VoidFunction EmptyVoidFunction() { return {}; }


// SequenceFunctions: returns a function that will call f1 and f2 in sequence
VoidFunction SequenceFunctions(const VoidFunction& f1, const VoidFunction& f2);


// AnyEventCallback can hold any bool(void *) function.
using AnyEventCallback = std::function<bool(void * backendEvent)>;
inline AnyEventCallback EmptyEventCallback() {return {}; }

// ConfirmExitCallback can hold any bool(void) function (return true to allow exit).
using ConfirmExitCallback = std::function<bool(void)>;
inline ConfirmExitCallback EmptyConfirmExitCallback() { return {}; }

// @@md


// --------------------------------------------------------------------------------------------------------------------

// @@md#MobileCallbacks

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


// --------------------------------------------------------------------------------------------------------------------

// @@md#EdgeToolbar

// EdgeToolbarType: location of an Edge Toolbar
enum class EdgeToolbarType
{
    Top,
    Bottom,
    Left,
    Right
};

struct EdgeToolbarOptions
{
    // height or width the top toolbar, in em units
    // (i.e. multiples of the default font size, to be Dpi aware)
    float sizeEm = 2.5f;

    // Padding inside the window, in em units
    ImVec2 WindowPaddingEm = ImVec2(0.3f, 0.3f);

    // Window background color, only used if WindowBg.w > 0
    ImVec4 WindowBg = ImVec4(0.f, 0.f, 0.f, 0.f);
};


// EdgeToolbar :a toolbar that can be placed on the edges of the App window
// It will be placed in a non-dockable window
struct EdgeToolbar
{
    VoidFunction ShowToolbar = EmptyVoidFunction();
    EdgeToolbarOptions options;
};

std::vector<EdgeToolbarType> AllEdgeToolbarTypes();
std::string EdgeToolbarTypeName(EdgeToolbarType e);
// @@md


// --------------------------------------------------------------------------------------------------------------------

// @@md#DefaultIconFont

// HelloImGui can optionally merge an icon font (FontAwesome 4 or 6) to the default font
// - you need to include manually icons_font_awesome_4.h or icons_font_awesome_6.h:
//     #include "hello_imgui/icons_font_awesome_6.h" or #include "hello_imgui/icons_font_awesome_4.h"
enum class DefaultIconFont
{
    NoIcons,
    FontAwesome4,
    FontAwesome6
};
// @@md


// --------------------------------------------------------------------------------------------------------------------

// @@md#RunnerCallbacks
// RunnerCallbacks is a struct that contains the callbacks
// that are called by the application
//
struct RunnerCallbacks
{
    // --------------- GUI Callbacks -------------------

    // `ShowGui`: Fill it with a function that will add your widgets.
    // (ShowGui will be called at each frame, before rendering the Dockable windows, if any)
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

    // `EdgesToolbars`:
    // A dict that contains toolbars that can be placed on the edges of the App window
    std::map<EdgeToolbarType, EdgeToolbar> edgesToolbars;

    // `AddEdgeToolbar`: Add a toolbar that can be placed on the edges of the App window
    void AddEdgeToolbar(EdgeToolbarType edgeToolbarType,
                        VoidFunction guiFunction,
                        const EdgeToolbarOptions& options = EdgeToolbarOptions());


    // --------------- Startup sequence callbacks -------------------

    // `PostInit_AddPlatformBackendCallbacks`:
    //  You can here add a function that will be called once after OpenGL and ImGui are inited,
    //  but before the platform backend callbacks are initialized.
    //  If you, want to add your own glfw callbacks, you should use this function to do so
    //  (and then ImGui will call your callbacks followed by its own callbacks)
    VoidFunction PostInit_AddPlatformBackendCallbacks = EmptyVoidFunction();


    // `PostInit`: You can here add a function that will be called once after everything
    //  is inited (ImGui, Platform and Renderer Backend)
    VoidFunction PostInit = EmptyVoidFunction();

    // `EnqueuePostInit`: Add a function that will be called once after OpenGL
    //  and ImGui are inited, but before the backend callback are initialized.
    //  (this will modify the `PostInit` callback by appending the new callback (using `SequenceFunctions`)
    void EnqueuePostInit(const VoidFunction& callback);

    // `LoadAdditionalFonts`: default=_LoadDefaultFont_WithFontAwesome*.
    //  A function that is called in order to load fonts.
    // `LoadAdditionalFonts` will be called once, then *set to nullptr*.
    // If you want to load additional fonts, during the app execution, you can
    // set LoadAdditionalFonts to a function that will load the additional fonts.
    VoidFunction LoadAdditionalFonts = (VoidFunction)ImGuiDefaultSettings::LoadDefaultFont_WithFontAwesomeIcons;
    // If LoadAdditionalFonts==LoadDefaultFont_WithFontAwesomeIcons, this parameter control
    // which icon font will be loaded by default.
    DefaultIconFont defaultIconFont = DefaultIconFont::FontAwesome4;

    // `SetupImGuiConfig`: default=_ImGuiDefaultSettings::SetupDefaultImGuiConfig*.
    //  If needed, change ImGui config via SetupImGuiConfig
    //  (enable docking, gamepad, etc)
    VoidFunction SetupImGuiConfig = (VoidFunction)ImGuiDefaultSettings::SetupDefaultImGuiConfig;

    // `SetupImGuiStyle`: default=_ImGuiDefaultSettings::SetupDefaultImGuiConfig*.
    //  If needed, set your own style by providing your own SetupImGuiStyle callback
    VoidFunction SetupImGuiStyle = (VoidFunction)ImGuiDefaultSettings::SetupDefaultImGuiStyle;

    // `RegisterTests`: A function that is called once ImGuiTestEngine is ready
    // to be filled with tests and automations definitions.
    VoidFunction RegisterTests = EmptyVoidFunction();
    // `registerTestsCalled`: will be set to true when RegisterTests was called
    // (you can set this to false if you want to RegisterTests to be called again
    //  during the app execution)
    bool         registerTestsCalled = false;


    // --------------- Exit sequence callbacks -------------------

    // `ConfirmExit`: Called when the user requests to close the window
    //  (window close button, Cmd-Q / Alt-F4, or the default App/Quit menu).
    //  Return true to proceed with the exit, false to cancel it (the app keeps running).
    //  Use it to confirm quitting (e.g. unsaved changes, or several open documents/tabs).
    //
    //  Two ways to use it:
    //    - Synchronous: pop a native/OS dialog and return the user's answer directly.
    //    - Deferred (to show a Dear ImGui modal): return false to cancel now, raise a
    //      flag, open your popup inside ShowGui, and set RunnerParams.appShallExit = true
    //      yourself once the user confirms there.
    //
    //  IMPORTANT: this is called during event polling, *before* ImGui::NewFrame().
    //  Do NOT call any ImGui function inside it. It is NOT called for programmatic
    //  exits (setting RunnerParams.appShallExit = true directly is always honored).
    //  Default: empty -> the exit proceeds immediately (the historical behavior).
    ConfirmExitCallback ConfirmExit = EmptyConfirmExitCallback();

    // `BeforeExit`: You can here add a function that will be called once before exiting
    //  (when OpenGL and ImGui are still inited)
    VoidFunction BeforeExit = EmptyVoidFunction();

    // `EnqueueBeforeExit`: Add a function that will be called once before exiting
    //  (when OpenGL and ImGui are still inited)
    // (this will modify the `BeforeExit` callback by appending the new callback (using `SequenceFunctions`)
    void EnqueueBeforeExit(const VoidFunction& callback);

    // `BeforeExit_PostCleanup`: You can here add a function that will be called once
    // before exiting (after OpenGL and ImGui have been stopped)
    VoidFunction BeforeExit_PostCleanup = EmptyVoidFunction();


    // --------------- Callbacks in the render loop -------------------

    // `PreNewFrame`: You can here add a function that will be called at each frame,
    //  and before the call to ImGui::NewFrame().
    //  It is a good place to add new dockable windows.
    VoidFunction PreNewFrame = EmptyVoidFunction();

    // `PostNewFrame`: You can here add a function that will be called at each frame,
    //  just after the call to ImGui::NewFrame(), and before any Gui code.
    VoidFunction PostNewFrame = EmptyVoidFunction();

    // `BeforeImGuiRender`: You can here add a function that will be called at each frame,
    //  after the user Gui code, and just before the call to
    //  ImGui::Render() (which will also call ImGui::EndFrame()).
    VoidFunction BeforeImGuiRender = EmptyVoidFunction();

    // `BeforeSwap`: You can here add a function that will be called at each frame,
    //  after the Gui was rendered, but before it is swapped to the screen.
    //  This is a good place for a full-screen post-process pass over the whole frame
    //  (for example a color management pass): unlike with AfterSwap, what you draw
    //  here is still part of the presented frame.
    //  Note: with OpenGL, Metal and DirectX11, the frame is still open at this point,
    //   so that you can draw into it. With Vulkan and DirectX12, the command buffer
    //   was already submitted by then: you would need to submit and synchronize
    //   your own work.
    //  Note: this concerns the main window only: the secondary viewport windows
    //   are rendered after this callback.
    VoidFunction BeforeSwap = EmptyVoidFunction();

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

    // `PostRenderDockableWindows`: Fill it with a function that will be called
    // after the dockable windows are rendered.
    VoidFunction PostRenderDockableWindows = EmptyVoidFunction();

    // `ThemeChanged`: You can here add a function that will be called
    //  immediately after `ImGuiTheme::ApplyTheme` has been executed. This is
    //  typically triggered when the user clicks a theme menu item in the menubar,
    //  allowing custom drawings or UI elements to update their colors right after
    //  the theme change.
    VoidFunction ThemeChanged = EmptyVoidFunction();

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


// AppendCallback: legacy synonym for SequenceFunctions
VoidFunction AppendCallback(const VoidFunction& previousCallback, const VoidFunction& newCallback);

}  // namespace HelloImGui
