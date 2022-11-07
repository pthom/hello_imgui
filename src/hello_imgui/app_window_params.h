#pragma once
#include "imgui.h"
#include "hello_imgui/screen_bounds.h"
#include <string>

/**
@@md#AppWindowParams

__AppWindowParams__ is a struct that defines the application window display params.
See [doc_src/hello_imgui_diagram.png](https://raw.githubusercontent.com/pthom/hello_imgui/master/src/hello_imgui/doc_src/hello_imgui_diagram.png)
for details.

Members:
* `windowTitle`: _string, default=""_. Title of the application window
* `windowGeometry`: _WindowGeometry_
  Enables to precisely set the window geometry (position, monitor, size, full screen, fake full screen, etc.)
   _Note: on a mobile device, the application will always be full screen._
* `restorePreviousGeometry`: _bool, default=false_.
  If true, then save & restore windowGeometry from last run

* `borderless`: _bool, default = false_.
* `resizable`: _bool, default = false_.
* `windowSizeState`: _WindowSizeState, default = Standard_ (minimized, maximized or standard)

@@md
**/


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


struct WindowGeometry
{
    // used if fullScreenMode==NoFullScreen and sizeAuto==false
    ScreenSize size = ScreenSize{100, 100};

    // If true, adapt the app window size to the presented widgets
    bool sizeAuto = false;

    FullScreenMode fullScreenMode = FullScreenMode::NoFullScreen;

    WindowPositionMode positionMode;

    // used if windowPositionMode==FromCoords
    ScreenPosition position = ScreenPosition{0, 0};

    // used if full screen, or if positionMode==MonitorCenter
    int monitorIdx = 0;
};


struct AppWindowParams
{
    std::string windowTitle;

    WindowGeometry windowGeometry;
    // if true, then save & restore from last run
    bool restorePreviousGeometry;

    bool borderless = false;
    bool resizable = true;
    WindowSizeState windowSizeState = WindowSizeState::Standard;
};

}  // namespace HelloImGui