#pragma once
#include "hello_imgui/app_window_params.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/backend_window_helper.h"

#include <optional>


namespace HelloImGui
{
    class WindowGeometryHelper
    {
    public:
        WindowGeometry& mGeometry;
        bool mRestoreLast;
        std::string mWindowGeometryIniFilename;

        WindowGeometryHelper(WindowGeometry &geometry, bool restoreLast, std::string windowGeometryIniFilename);

        bool HasInitialWindowSizeInfo() const;
        ScreenBounds AppWindowBoundsInitial(const std::vector<ScreenBounds>& allMonitorsWorkAreas);

        void EnsureWindowFitsMonitor(BackendApi::IBackendWindowHelper* helper, BackendApi::WindowPointer window, std::function<void()> fnBeforeChangingWindowBounds);
        void CenterWindowOnMonitor(BackendApi::IBackendWindowHelper* helper, BackendApi::WindowPointer window, std::function<void()> fnBeforeChangingWindowBounds);
        ScreenBounds GetCurrentMonitorWorkArea(BackendApi::IBackendWindowHelper* backendWindowHelper,
                                               BackendApi::WindowPointer window);

        // Will set the window size, after making sure it fits on the current screen
        void TrySetWindowSize(BackendApi::IBackendWindowHelper *backendWindowHelper, BackendApi::WindowPointer window, ImVec2 userWidgetsSize, std::function<void()> fnBeforeChangingWindowBounds);

    private:
        int GetMonitorIndexFromWindowPosition(BackendApi::IBackendWindowHelper *backendWindowHelper, const ScreenPosition& windowPosition) const;
    };

    struct SearchForMonitorResult
    {
        int monitorIdx;
        std::optional<ScreenPosition> newPosition;
    };
    SearchForMonitorResult SearchForMonitor(const std::vector<ScreenBounds>& monitorsWorkAreas, const AppWindowParams& appWindowParams);

}