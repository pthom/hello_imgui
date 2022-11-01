#pragma once
#include "imgui.h"
#include "hello_imgui/screen_bounds.h"
#include <string>

/**
@@md#AppWindowParams

__AppWindowParams__ is a struct that defines the application window display params.

Members:
* `windowTitle`: _string, default=""_. Title of the application window
* `windowGeometry`: _WindowGeometry_
  Enables to precisely set the window geometry (position, monitor, size, full screen, fake full screen, etc.)
   _Note: on a mobile device, the application will always be full screen._
  See [doc_src/hello_imgui_diagram.png](doc_src/hello_imgui_diagram.png) for details.
* `restorePreviousGeometry`: _bool, default=false_.
  If true, then save & restore windowGeometry from last run
* `windowAppearance`: _WindowAppearance_: set window decoration, and status (resizable, minimized, etc)

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


    struct WindowSize
    {
        // used if fullScreenMode==NoFullScreen and autoSize==false
        ScreenSize size = ScreenSize{100, 100};

        // If true, adapt the app window size to the widgets present in the window
        bool autoSize = false;

        FullScreenMode fullScreenMode = FullScreenMode::NoFullScreen;
    };


    enum class WindowPositionMode
    {
        OsDefault,
        MonitorCenter,
        FromCoords,
    };

    struct WindowPosition
    {
        WindowPositionMode windowPositionMode;

        // used if windowPositionMode==FromCoords
        ScreenPosition position = ScreenPosition{0, 0};
    };


    struct WindowGeometry
    {
        WindowSize windowSize;
        WindowPosition windowPosition;

        // used if full screen, or if windowPositionMode==MonitorCenter
        int monitorIdx = 0;
    };


    struct WindowAppearance
    {
        bool borderless = false;
        bool resizable = false;
        WindowSizeState windowSizeState = WindowSizeState::Standard;
    };


    struct AppWindowParams
    {
        std::string windowTitle;

        WindowGeometry windowGeometry;
        // if true, then save & restore from last run
        bool restorePreviousGeometry;

        WindowAppearance windowAppearance;
    };

struct AppWindowParamsOld
{
    AppWindowParamsOld(std::string windowTitle_ = "",
                       ImVec2 windowSize_ = {800.f, 600.f},
                       bool maximized_ = false,
                       bool fullScreen_ = false,
                       ImVec2 windowPosition_ = {-11000.f, -1.f})
        : windowTitle(windowTitle_),
          windowSize(windowSize_),
          maximized(maximized_),
          fullScreen(fullScreen_),
          windowPosition(windowPosition_)
    {
    }
    
    std::string windowTitle = "";
    ImVec2 windowSize = {800.f, 600.f};
    bool maximized = false;
    bool fullScreen = false;
    ImVec2 windowPosition = {-11000.f, -1.f};
};

}  // namespace HelloImGui