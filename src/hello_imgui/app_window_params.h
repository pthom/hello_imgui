#pragma once
#include "imgui.h"
#include <string>

namespace HelloImGui
{
/**
@@md#AppWindowParams

    __AppWindowParams__ is a struct that defines the application window display params.

    Members:
    * `windowTitle`: _string, default=""_. Title of the application window
    * `windowSize`: _ImVec2, default (800,600)_. Size of the window.
    * `maximized`: _bool, default=false_. If this boolean flag is true, the application window
       will occupy the full space of the primary screen
    * `fullScreen`: _bool, default=false_. If this boolean flag is true, the application window
       will be full screen, with no decorations.
        _Note: on a mobile device, the application will always be full screen._
    * `windowPosition`: _ImVec2, default=(-11000, -1)_. Position of the window if x >= -1000,
       else let the OS decide

@@md
**/
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
    
    std::string windowTitle = "";
    ImVec2 windowSize = {800.f, 600.f};
    bool maximized = false;
    bool fullScreen = false;
    ImVec2 windowPosition = {-11000.f, -1.f};
};

}  // namespace HelloImGui