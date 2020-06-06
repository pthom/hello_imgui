#pragma once
#include "imgui.h"
#include <string>

namespace HelloImGui
{
/// AppWindowParams: defines the application window display params
/// Note for mobile apps: Mobile apps are always full screen, so that these
/// params will not be taken into account
struct AppWindowParams
{
    AppWindowParams(std::string windowTitle_ = "",
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

    /// title of the application window (empty by default)
    std::string windowTitle = "";

    /// size of the window (default 800x600)
    ImVec2 windowSize = {800.f, 600.f};

    /// If this boolean flag is true, the application window will occupy the full space of the primary screen
    bool maximized = false;

    /// If this boolean flag is true, the application window will be full screen, with no decorations.
    /// _Note: on a mobile device, the application will always be full screen._
    bool fullScreen = false;

    /// position of the window (if x <= -10000., then let the OS decide)
    ImVec2 windowPosition = {-11000.f, -1.f};
};

}  // namespace HelloImGui