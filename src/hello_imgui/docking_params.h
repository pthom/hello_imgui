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

using DockSpaceId = std::string;
using WindowId = std::string;
struct DockingSplit
{
    /// id of the space that should be split :
    /// At the start, there is only one space, named "MainDockSpace"
    DockSpaceId initialDock;
    DockSpaceId newDock;
    ImGuiDir_ direction;
    float ratio;
};

struct DockableWindowInfo {
    std::string label = "";
    bool isVisible = true;
    bool displayCloseButton = true;


    GuiFunctionPointer guiFonction = {};
};



struct DockingParams
{
    std::vector<DockingSplit> dockingSplits;
    std::vector<std::pair<WindowId, DockSpaceId>>  windowDockingLocations;

    /// Call this function from the code in order to restore the dock layout
    /// to the initial layout provided by InitialDockLayoutFunction
    inline void ResetDockLayout() { wasDockLayoutApplied = false; }

    /// wasDockLayoutApplied is an internal variable (set by InitialDockLayoutFunction() and ResetDockLayout())
    bool wasDockLayoutApplied = false;

    /// Restores the initial layout (and remove imgui.ini, since it could override the programmatic layout)
    inline void ForceResetLayout()
    {
        remove("imgui.ini");
        wasDockLayoutApplied = false;
    }
};
}