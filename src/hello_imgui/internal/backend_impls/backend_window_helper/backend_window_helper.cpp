#include "backend_window_helper.h"


namespace HelloImGui { namespace BackendApi
{
    IBackendWindowHelper::SearchForMonitorResult IBackendWindowHelper::SearchForMonitor(
        const WindowGeometry& geometry)
    {
        IBackendWindowHelper::SearchForMonitorResult r;

        //
        // Search for corresponding monitor
        // - Will set r.monitorIdx
        // - May set r.newPosition if out of bounds
        //
        if (geometry.positionMode != WindowPositionMode::FromCoords)
        {
            r.monitorIdx = geometry.monitorIdx;
        }
        else
        {
            // If position from coords, search for screen containing the window position
            int foundMonitorIdx = -1;
            size_t nbMonitors = GetNbMonitors();
            auto& wantedPosition = geometry.position;
            for (size_t monitorIdx = 0; monitorIdx < nbMonitors; ++monitorIdx)
            {
                auto workArea = GetOneMonitorWorkArea(monitorIdx);
                if (workArea.Contains(wantedPosition))
                    foundMonitorIdx = monitorIdx;
            }

            // If the given position is not on any screen, move the window to the primary screen
            if (foundMonitorIdx < 0)
            {
                auto workArea = GetOneMonitorWorkArea(0);
                r.newPosition = workArea.position;
                foundMonitorIdx = 0;
            }

            r.monitorIdx = foundMonitorIdx;
        }

        assert((r.monitorIdx >= 0) && (r.monitorIdx < GetNbMonitors()));

        return r;
    }


    void IBackendWindowHelper::EnsureWindowFitsMonitor(WindowPointer window, int monitorIdx, FullScreenMode fullScreenMode)
    {
        assert((monitorIdx >= 0) && (monitorIdx < GetNbMonitors()));

        bool isRealFullScreen = (fullScreenMode == FullScreenMode::FullScreenDesktopResolution)
                                || (fullScreenMode == FullScreenMode::FullScreen);
        if (isRealFullScreen)
            return;

        auto windowBounds = GetWindowBounds(window);
        auto monitorBounds = GetOneMonitorWorkArea(monitorIdx);

        bool changed = false;
        if (! monitorBounds.Contains(windowBounds.position))
        {
            windowBounds.position = monitorBounds.position;
            changed = true;
        }
        if (! monitorBounds.Contains(windowBounds.BottomRightCorner()))
        {
            changed = true;
            windowBounds.position = monitorBounds.position;
            if (! monitorBounds.Contains(windowBounds.BottomRightCorner()))
            {
                windowBounds.size = monitorBounds.size;
            }
        }

        if (changed)
            SetWindowBounds(window, windowBounds);
    }

}}
