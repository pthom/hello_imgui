#pragma once
#include "imgui.h"
#include <functional>
#include <stdio.h>

namespace HelloImGui
{
struct DockingParams
{
    /// Callback that will create the initial docking layout
    std::function<void(ImGuiID /* fullDockSpace */)> InitialDockLayoutFunction = nullptr;
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