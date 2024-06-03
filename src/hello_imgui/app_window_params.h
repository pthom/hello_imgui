#pragma once
#include "imgui.h"
#include "hello_imgui/screen_bounds.h"
#include <string>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

namespace HelloImGui
{
enum class FullScreenMode
{
    NoFullScreen,
    FullScreen,                    // Full screen with specified resolution
    FullScreenDesktopResolution,   // Full screen with current desktop mode & resolution
    FullMonitorWorkArea            // Fake full screen, maximized window on the selected monitor
};


enum class WindowSizeState
{
    Standard,
    Minimized,
    Maximized
};


enum class WindowPositionMode
{
    OsDefault,
    MonitorCenter,
    FromCoords,
};


enum class WindowSizeMeasureMode
{
    // ScreenCoords: measure window size in screen coords.
    //     Note: screen coordinates *might* differ from real pixel on high dpi screens; but this depends on the OS.
    //         - For example, on apple a retina screenpixel size 3456x2052 might be seen as 1728x1026 in screen
    //           coordinates
    //         - Under windows, and if the application is DPI aware, ScreenCoordinates correspond to real pixels, 
    //           even on high density screens
    ScreenCoords,

    // RelativeTo96Ppi enables to give screen size that are independant from the screen density.
    // For example, a window size expressed as 800x600 will correspond to a size
    //    800x600 (in screen coords) if the monitor dpi is 96
    //    1600x120 (in screen coords) if the monitor dpi is 192
    RelativeTo96Ppi
};



// @@md#WindowGeometry
//
// WindowGeometry is a struct that defines the window geometry.
struct WindowGeometry
{
    // --------------- Window Size ------------------

    // Size of the application window
    // used if fullScreenMode==NoFullScreen and sizeAuto==false. Default=(800, 600)
    // The size will be handled as if it was specified for a 96PPI screen
    // (i.e. a given size will correspond to the same physical size on different screens, whatever their DPI)
    ScreenSize size = DefaultWindowSize;

    // If sizeAuto=true, adapt the app window size to the presented widgets.
    // After the first frame was displayed, HelloImGui will measure its size, and the
    // application window will be resized.
    // As a consequence, the application window may change between the 1st and 2nd frame.
    // If true, adapt the app window size to the presented widgets. This is done at startup
    bool sizeAuto = false;

    // `windowSizeState`: _WindowSizeState, default=Standard_
    //  You can choose between several window size states:
    //      Standard,
    //      Minimized,
    //      Maximized
    WindowSizeState windowSizeState = WindowSizeState::Standard;

    // `windowSizeMeasureMode`: _WindowSizeMeasureMode_, default=RelativeTo96Ppi
    // Define how the window size is specified:
    //      * RelativeTo96Ppi enables to give a screen size whose physical result
    //      (in millimeters) is independent of the screen density.
    //         For example, a window size expressed as 800x600 will correspond to a size
    //            - 800x600 (in screen coords) if the monitor dpi is 96
    //            - 1600x120 (in screen coords) if the monitor dpi is 192
    //          (this works with Glfw. With SDL, it only works under windows)
    //      * ScreenCoords: measure window size in screen coords
    //        (Note: screen coordinates might differ from real pixels on high dpi screen)
    WindowSizeMeasureMode windowSizeMeasureMode = WindowSizeMeasureMode::RelativeTo96Ppi;


    // --------------- Position ------------------

    // `positionMode`: you can choose between several window position modes:
    //      OsDefault,
    //      MonitorCenter,
    //      FromCoords,
    WindowPositionMode positionMode = WindowPositionMode::OsDefault;

    // `position`: used if windowPositionMode==FromCoords, default=(40, 40)
    ScreenPosition position = DefaultScreenPosition;

    // `monitorIdx`: index of the monitor to use, default=0
    //  used if positionMode==MonitorCenter or if fullScreenMode!=NoFullScreen
    int monitorIdx = 0;


    // --------------- Full screen ------------------

    // `fullScreenMode`: you can choose between several full screen modes:
    //      NoFullScreen,
    //      FullScreen,                  // Full screen with specified resolution
    //      FullScreenDesktopResolution, // Full screen with current desktop mode & resolution
    //      FullMonitorWorkArea          // Fake full screen (maximized window) on the selected monitor
    FullScreenMode fullScreenMode = FullScreenMode::NoFullScreen;


    // --------------- Auto Resize ------------------

    // `resizeAppWindowAtNextFrame`: _bool_, default=false;
    //  If you set this to flag to true at any point during the execution, the application
    //  window will then try to resize based on its content on the next displayed frame,
    //  and this flag will subsequently be set to false.
    //  Example:
    //   ```cpp
    //   // Will resize the app window at next displayed frame
    //   HelloImGui::GetRunnerParams()->appWindowParams.windowGeometry.resizeAppWindowAtNextFrame = true;
    //   ```
    //  Note: this flag is intended to be used during execution, not at startup
    //  (use sizeAuto at startup).
    bool resizeAppWindowAtNextFrame = false;
};
// @@md


// If there is a notch on the iPhone, you should not display inside these insets
struct EdgeInsets
{
    double top = 0.;     // Typically around 47
    double left = 0.;    // Typically 0
    double bottom = 0.;  // Typically around 34
    double right = 0.;   // Typically 0
};


// @@md#AppWindowParams
//
// AppWindowParams is a struct that defines the application window display params.
//See https://raw.githubusercontent.com/pthom/hello_imgui/master/src/hello_imgui/doc_src/hello_imgui_diagram.jpg
// for details.
struct AppWindowParams
{
    // --------------- Standard params ------------------

    // `windowTitle`: _string, default=""_. Title of the application window
    std::string windowTitle;

    // `windowGeometry`: _WindowGeometry_
    //  Enables to precisely set the window geometry (position, monitor, size,
    //  full screen, fake full screen, etc.)
    //   _Note: on a mobile device, the application will always be full screen._
    WindowGeometry windowGeometry;

    // `restorePreviousGeometry`: _bool, default=false_.
    // If true, then save & restore windowGeometry from last run (the geometry
    // will be written in imgui_app_window.ini)
    bool restorePreviousGeometry = false;

    // `resizable`: _bool, default = false_. Should the window be resizable.
    // This is taken into account at creation.
    bool resizable = true;
    // `hidden`: _bool, default = false_. Should the window be hidden.
    // This is taken into account dynamically (you can show/hide the window with this).
    // Full screen windows cannot be hidden.
    bool hidden = false;


    // --------------- Borderless window params ------------------

    // `borderless`: _bool, default = false_. Should the window have borders.
    // This is taken into account at creation.
    bool   borderless = false;
    // `borderlessMovable`: if the window is borderless, should it be movable.
    //   If so, a drag zone is displayed at the top of the window when the mouse is over it.
    bool   borderlessMovable = true;
    // `borderlessResizable`: if the window is borderless, should it be resizable.
    //  If so, a drag zone is displayed at the bottom-right of the window
    //  when the mouse is over it.
    bool   borderlessResizable = true;
    // `borderlessClosable`: if the window is borderless, should it have a close button.
    //  If so, a close button is displayed at the top-right of the window
    //  when the mouse is over it.
    bool   borderlessClosable = true;
    // `borderlessHighlightColor`:
    //   Color of the highlight displayed on resize/move zones.
    //   If borderlessHighlightColor.w==0, then the highlightColor will be automatically
    //   set to ImGui::GetColorU32(ImGuiCol_TitleBgActive, 0.6f)
    ImVec4 borderlessHighlightColor = ImVec4(0.2f, 0.4f, 1.f, 0.3f);


    // --------------- iOS Notch ------------------

    // `edgeInsets`: _EdgeInsets_. iOS only, out values filled by HelloImGui.
    // If there is a notch on the iPhone, you should not display inside these insets.
    // HelloImGui handles this automatically, if handleEdgeInsets is true and
    // if runnerParams.imGuiWindowParams.defaultImGuiWindowType is not NoDefaultWindow.
    // (warning, these values are updated only after a few frames,
    //  they are typically 0 for the first 4 frames)
    EdgeInsets edgeInsets;
    // `handleEdgeInsets`: _bool, default = true_. iOS only.
    // If true, HelloImGui will handle the edgeInsets on iOS.
    bool       handleEdgeInsets = true;

    // ----------------- repaint the window during resize -----------------
    // Very advanced and reserved for advanced C++ users.
    // If you set this to true, the window will be repainted during resize.
    // Do read https://github.com/pthom/hello_imgui/issues/112 for info about the possible gotchas
    // (This API is not stable, as the name suggests, and this is not supported)
    bool repaintDuringResize_GotchaReentrantRepaint = false;
};
// @@md

}  // namespace HelloImGui