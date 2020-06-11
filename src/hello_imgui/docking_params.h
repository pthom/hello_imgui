#pragma once
#include "imgui.h"
#include "hello_imgui/runner_callbacks.h"
#include <functional>
#include <string>
#include <vector>
#include <utility>
#include <stdio.h>

namespace HelloImGui
{
//StatusBar
//ToolBar
//Menu


//using LayoutMode = std::string;
//using DockableWindowId = std::string;
//

using DockSpaceName = std::string;
using WindowId = std::string;
struct DockingSplit
{
    /// id of the space that should be split :
    /// At the start, there is only one space, named "MainDockSpace"
    DockSpaceName initialDock;
    DockSpaceName newDock;
    ImGuiDir_ direction;
    float ratio;
};

struct DockableWindow
{
    DockableWindow(
        const std::string & label_,
        const DockSpaceName & dockSpaceName_,
        const GuiFunctionPointer guiFonction_,
        bool isVisible_ = true,
        bool displayCloseButton_ = true)
    : label(label_), dockSpaceName(dockSpaceName_), guiFonction(guiFonction_),
      isVisible(isVisible_), displayCloseButton(displayCloseButton_) {}

    std::string label;
    DockSpaceName dockSpaceName;
    GuiFunctionPointer guiFonction = {};

    bool isVisible = true;
    bool displayCloseButton = true;
};


struct DockingParams
{
    std::vector<DockingSplit> dockingSplits;
    //std::vector<std::pair<WindowId, DockSpaceName>>  windowDockingLocations;
    std::vector<DockableWindow> dockableWindows;

    /// Reset user layout at application startup?
    bool resetUserDockLayout = true;

    /// Call this function from the code in order to restore the dock layout
    /// to the initial layout provided by InitialDockLayoutFunction
    inline void ResetDockLayout() { wasDockLayoutApplied = false; }

    /// wasDockLayoutApplied is an internal variable (set by InitialDockLayoutFunction() and ResetDockLayout())
    bool wasDockLayoutApplied = false;
};
}