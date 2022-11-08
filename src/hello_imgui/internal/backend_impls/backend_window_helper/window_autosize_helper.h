#pragma once
#include "window_geometry_helper.h"

#include <optional>

namespace HelloImGui
{
    // This class can resize the app window to the size of the imgui widgets seen on the first frame
    class WindowAutoSizeHelper
    {
        WindowGeometryHelper& mWindowGeometryHelper;

        bool mFlagIsMeasuringSize = false;

    public:
        WindowAutoSizeHelper(WindowGeometryHelper& windowGeometryHelper);
        void BeginMeasureSize(BackendApi::IBackendWindowHelper* helper, BackendApi::WindowPointer window);
        void EndMeasureSize(BackendApi::IBackendWindowHelper* helper, BackendApi::WindowPointer window);
        void EnsureWindowFitsMonitor(BackendApi::IBackendWindowHelper* helper, BackendApi::WindowPointer window);
        bool WantAutoSize();

    private:
        void _ForceWindowSize(BackendApi::IBackendWindowHelper *backendWindowHelper, BackendApi::WindowPointer window);

        int GetMonitorIndexFromWindowPosition(BackendApi::IBackendWindowHelper *backendWindowHelper, const ScreenPosition& windowPosition);
        ScreenBounds GetCurrentMonitorWorkArea(BackendApi::IBackendWindowHelper *backendWindowHelper, BackendApi::WindowPointer window);

    };
}