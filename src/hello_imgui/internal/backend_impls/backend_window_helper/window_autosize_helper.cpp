#include "window_autosize_helper.h"

#include "imgui.h"

namespace HelloImGui
{
    WindowAutoSizeHelper::WindowAutoSizeHelper(WindowGeometryHelper& windowGeometryHelper)
        : mWindowGeometryHelper(windowGeometryHelper)
    {
    }

    void WindowAutoSizeHelper::_ForceWindowSize(BackendApi::IBackendWindowHelper *backendWindowHelper, BackendApi::WindowPointer window)
    {
        auto userWidgetsSize = ImGui::GetItemRectSize();
        int widgetsMargin = 6;

        auto screenSize = GetCurrentMonitorWorkArea(backendWindowHelper, window).size;

        auto computedSize = ScreenSize {
            std::min((int)userWidgetsSize.x + widgetsMargin, screenSize[0]),
            std::min((int)userWidgetsSize.y + widgetsMargin, screenSize[1])
        };

        auto windowBounds = backendWindowHelper->GetWindowBounds(window);
        windowBounds.size = computedSize;

        backendWindowHelper->SetWindowBounds(window, windowBounds);
    }

    ScreenBounds WindowAutoSizeHelper::GetCurrentMonitorWorkArea(BackendApi::IBackendWindowHelper *backendWindowHelper, BackendApi::WindowPointer window)
    {
        auto windowBounds = backendWindowHelper->GetWindowBounds(window);
        int monitorIdx = GetMonitorIndexFromWindowPosition(backendWindowHelper, windowBounds.position);
        auto areas = backendWindowHelper->GetMonitorsWorkAreas();
        auto r = areas[monitorIdx];
        return r;
    }

    void WindowAutoSizeHelper::BeginMeasureSize(BackendApi::IBackendWindowHelper *, BackendApi::WindowPointer )
    {
        if (WantAutoSize())
        {
            mFlagIsMeasuringSize = true;
            ImGui::BeginGroup();
        }
        else
        {
            mFlagIsMeasuringSize = false;
        }
    }

    void WindowAutoSizeHelper::EndMeasureSize(BackendApi::IBackendWindowHelper *backendWindowHelper, BackendApi::WindowPointer window)
    {
        if (mFlagIsMeasuringSize)
        {
            ImGui::EndGroup();
            _ForceWindowSize(backendWindowHelper, window);
        }
    }

    int WindowAutoSizeHelper::GetMonitorIndexFromWindowPosition(BackendApi::IBackendWindowHelper *backendWindowHelper, const ScreenPosition& windowPosition)
    {
        const auto& geometry = mWindowGeometryHelper.mGeometry;

        if (geometry.fullScreenMode != FullScreenMode::NoFullScreen)
            return geometry.monitorIdx;

        auto monitorsWorkAreas = backendWindowHelper->GetMonitorsWorkAreas();

        for (int monitorIdx = 0; monitorIdx < monitorsWorkAreas.size(); monitorIdx++)
        {
            auto monitor_work_area = monitorsWorkAreas[monitorIdx];
            if (monitor_work_area.Contains(windowPosition))
                return monitorIdx;
        }

        // Handle failure and lost windows:
        // if we did not find any monitor containing the window position, return the closest monitor
        int minDistance = 1000000;
        int bestMonitorIdx = -1;
        for (int monitorIdx = 0; monitorIdx < monitorsWorkAreas.size(); monitorIdx++)
        {
            auto monitor_work_area = monitorsWorkAreas[monitorIdx];
            int distance = monitor_work_area.DistanceFromPixel(windowPosition);
            if (distance < minDistance)
            {
                minDistance = distance;
                bestMonitorIdx = monitorIdx;
            }
        }

        return bestMonitorIdx;
    }

    void WindowAutoSizeHelper::EnsureWindowFitsMonitor(BackendApi::IBackendWindowHelper *backendWindowHelper, BackendApi::WindowPointer window)
    {
        auto currentMonitorWorkArea = GetCurrentMonitorWorkArea(backendWindowHelper, window);
        auto currentWindowBounds = backendWindowHelper->GetWindowBounds(window);
        auto currentWindowBoundsNew = currentMonitorWorkArea.EnsureWindowFitsThisMonitor(currentWindowBounds);
        if ( !(currentWindowBoundsNew == currentWindowBounds))
        {
            backendWindowHelper->SetWindowBounds(window, currentWindowBoundsNew);
        }
    }

    bool WindowAutoSizeHelper::WantAutoSize()
    {
        return ! mWindowGeometryHelper.HasWindowSizeInfo();
    }

}