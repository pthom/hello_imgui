#pragma once
#include "window_geometry_helper.h"

#include <optional>

namespace HelloImGui
{
    // This class can resize the app window to the size of the imgui widgets seen on the first frame
    class WindowAutoSizeHelper
    {
        WindowGeometryHelper& mWindowGeometryHelper;

    public:
        WindowAutoSizeHelper(WindowGeometryHelper& windowGeometryHelper);
        void EnsureWindowFitsMonitor(BackendApi::IBackendWindowHelper* helper, BackendApi::WindowPointer window);
        void CenterWindowOnMonitor(BackendApi::IBackendWindowHelper* helper, BackendApi::WindowPointer window);
        ScreenBounds GetCurrentMonitorWorkArea(BackendApi::IBackendWindowHelper* backendWindowHelper,
                                               BackendApi::WindowPointer window);

        // Will set the window size, after making sure it fits on the current screen
        void TrySetWindowSize(BackendApi::IBackendWindowHelper *backendWindowHelper, BackendApi::WindowPointer window, ImVec2 userWidgetsSize);

    private:

        int GetMonitorIndexFromWindowPosition(BackendApi::IBackendWindowHelper *backendWindowHelper, const ScreenPosition& windowPosition);

    };
}