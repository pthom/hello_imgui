#pragma once
#include "hello_imgui/app_window_params.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/backend_window_helper.h"

#include <optional>


// Fixme: window_geometry_helper and window_autosize_helper should be merged
namespace HelloImGui
{
    class WindowGeometryHelper
    {
    public:
        WindowGeometry mGeometry;
        bool mRestoreLast;

        WindowGeometryHelper(WindowGeometry &geometry, bool restoreLast);

        bool HasInitialWindowSizeInfo();

        ScreenBounds AppWindowBoundsInitial(const std::vector<ScreenBounds>& allMonitorsWorkAreas);

        static void WriteLastRunWindowBounds(const ScreenBounds& windowBounds);
        static std::optional<ScreenBounds> ReadLastRunWindowBounds();
    };

    struct SearchForMonitorResult
    {
        int monitorIdx;
        std::optional<ScreenPosition> newPosition;
    };
    SearchForMonitorResult SearchForMonitor(const std::vector<ScreenBounds>& monitorsWorkAreas, const AppWindowParams& appWindowParams);

}