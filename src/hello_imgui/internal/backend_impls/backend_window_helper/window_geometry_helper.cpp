#include "window_geometry_helper.h"
#include "hello_imgui/internal/hello_imgui_ini_settings.h"

#include <vector>
#include <string>
#include <cassert>


namespace HelloImGui
{
    WindowGeometryHelper::WindowGeometryHelper(WindowGeometry &geometry, bool restoreLast, std::string windowGeometryIniFilename) :
        mGeometry(geometry), mRestoreLast(restoreLast), mWindowGeometryIniFilename(std::move(windowGeometryIniFilename))
        {}

    bool WindowGeometryHelper::HasInitialWindowSizeInfo() const
    {
        if (
            (mGeometry.windowSizeState == WindowSizeState::Maximized)
            || (mGeometry.fullScreenMode != FullScreenMode::NoFullScreen))
        {
            return true;
        }
        if (!mGeometry.sizeAuto)
            return true;
        if (mRestoreLast && HelloImGuiIniSettings::LoadLastRunWindowBounds(mWindowGeometryIniFilename).has_value())
            return true;

        return false;
    }

    ScreenBounds WindowGeometryHelper::AppWindowBoundsInitial(const std::vector<ScreenBounds>& allMonitorsWorkAreas)
    {
        assert(mGeometry.monitorIdx >= 0);
        if (mGeometry.monitorIdx >= allMonitorsWorkAreas.size())
        {
            fprintf(stderr, "HelloImGui: could not use non existent monitor #%i!\n", mGeometry.monitorIdx);
            mGeometry.monitorIdx = 0;
        }

        // if maximized, use the full work area of the selected monitor
        if (mGeometry.windowSizeState == WindowSizeState::Maximized)
        {
            auto monitorBounds = allMonitorsWorkAreas[mGeometry.monitorIdx];
            return monitorBounds;
        }

        // if full screen, let the backend handle the positioning, but pass the size
        // so that it can change the resolution if required
        if (mGeometry.fullScreenMode != FullScreenMode::NoFullScreen)
        {
            // auto monitorBounds = allMonitorsWorkAreas[mGeometry.monitorIdx];
            // return WindowBounds(None, self.imgui_app_params.app_window_size)
            ScreenBounds r;
            r.size = mGeometry.size;
            return r;
        }

        //
        // Standard windowed mode
        //
        std::optional<ScreenBounds> windowBoundsLastRun;
        if (mRestoreLast)
          windowBoundsLastRun = HelloImGuiIniSettings::LoadLastRunWindowBounds(mWindowGeometryIniFilename);

        // Window Size
        auto computeSize = [&]() -> ScreenSize
        {
            if (mRestoreLast && windowBoundsLastRun.has_value())
                return windowBoundsLastRun->size;
            if (!mGeometry.sizeAuto)
                return mGeometry.size;

            return ScreenSize{150, 150};
        };
        auto windowSize = computeSize();

        // Window position
        auto centerWindowOnMonitor = [&]() -> ScreenPosition {
            auto monitorIdx = mGeometry.monitorIdx;
            auto pos = allMonitorsWorkAreas[monitorIdx].WinPositionCentered(windowSize);
            return pos;
        };

        auto computeWindowPosition = [&]() -> ScreenPosition {
            if (mGeometry.positionMode == WindowPositionMode::FromCoords)
                return mGeometry.position;
            if (mRestoreLast && windowBoundsLastRun.has_value())
                return windowBoundsLastRun->position;
            else
                return centerWindowOnMonitor();
        };

        auto windowPosition = computeWindowPosition();

        auto windowBoundsInitial = ScreenBounds{windowPosition, windowSize};
        return windowBoundsInitial;
    }


    SearchForMonitorResult SearchForMonitor(
        const std::vector<ScreenBounds>& monitorsWorkAreas, const AppWindowParams& appWindowParams)
    {
        SearchForMonitorResult r;

        const auto& geometry = appWindowParams.windowGeometry;

        bool isAnyFullScreen = (geometry.fullScreenMode != HelloImGui::FullScreenMode::NoFullScreen);

        //
        // Search for corresponding monitor
        // - Will set r.monitorIdx
        // - May set r.newPosition if out of bounds
        //
        if (isAnyFullScreen || (geometry.positionMode != WindowPositionMode::FromCoords))
        {
            r.monitorIdx = geometry.monitorIdx;
            assert((r.monitorIdx >= 0) && (r.monitorIdx < monitorsWorkAreas.size()));
        }
        else
        {
            // If position from coords, search for screen containing the window position
            int foundMonitorIdx = -1;
            assert(!monitorsWorkAreas.empty());
            auto& wantedPosition = geometry.position;
            for (size_t monitorIdx = 0; monitorIdx < monitorsWorkAreas.size(); ++monitorIdx)
            {
                auto workArea = monitorsWorkAreas[monitorIdx];
                if (workArea.Contains(wantedPosition))
                    foundMonitorIdx = (int)monitorIdx;
            }

            // If the given position is not on any screen, move the window to the primary screen
            if (foundMonitorIdx < 0)
            {
                auto workArea = monitorsWorkAreas[0];
                r.newPosition = workArea.position;
                foundMonitorIdx = 0;
            }

            r.monitorIdx = foundMonitorIdx;
        }

        return r;
    }



    ///////////////////////////////


    void WindowGeometryHelper::TrySetWindowSize(
        BackendApi::IBackendWindowHelper *backendWindowHelper, BackendApi::WindowPointer window, ImVec2 userWidgetsSize,
        std::function<void()> fnBeforeChangingWindowBounds)
    {
        int widgetsMargin = 6;

        auto screenSize = GetCurrentMonitorWorkArea(backendWindowHelper, window).size;

        auto computedSize = ScreenSize {
            std::min((int)userWidgetsSize.x + widgetsMargin, screenSize[0]),
            std::min((int)userWidgetsSize.y + widgetsMargin, screenSize[1])
        };

        auto windowBounds = backendWindowHelper->GetWindowBounds(window);
        windowBounds.size = computedSize;

        fnBeforeChangingWindowBounds();
        backendWindowHelper->SetWindowBounds(window, windowBounds);
    }

    ScreenBounds WindowGeometryHelper::GetCurrentMonitorWorkArea(BackendApi::IBackendWindowHelper *backendWindowHelper, BackendApi::WindowPointer window)
    {
        auto windowBounds = backendWindowHelper->GetWindowBounds(window);
        int monitorIdx = GetMonitorIndexFromWindowPosition(backendWindowHelper, windowBounds.position);
        auto areas = backendWindowHelper->GetMonitorsWorkAreas();
        auto r = areas[monitorIdx];
        return r;
    }

    int WindowGeometryHelper::GetMonitorIndexFromWindowPosition(BackendApi::IBackendWindowHelper *backendWindowHelper, const ScreenPosition& windowPosition) const
    {
        if (mGeometry.fullScreenMode != FullScreenMode::NoFullScreen)
            return mGeometry.monitorIdx;

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

    void WindowGeometryHelper::EnsureWindowFitsMonitor(
        BackendApi::IBackendWindowHelper *backendWindowHelper, BackendApi::WindowPointer window,
        std::function<void()> fnBeforeChangingWindowBounds)
    {
        auto currentMonitorWorkArea = GetCurrentMonitorWorkArea(backendWindowHelper, window);

#ifdef _WIN32
        currentMonitorWorkArea.position[1] += 35;  // Because windows are unmovable if their top edge if out of screen
        currentMonitorWorkArea.size[1] -= 35;  // Because the windows start bar is huge and not handled by glfwGetMonitorWorkarea
#endif

        auto currentWindowBounds = backendWindowHelper->GetWindowBounds(window);
        auto currentWindowBoundsNew = currentMonitorWorkArea.EnsureWindowFitsThisMonitor(currentWindowBounds);
        if ( !(currentWindowBoundsNew == currentWindowBounds))
        {
            fnBeforeChangingWindowBounds();
            backendWindowHelper->SetWindowBounds(window, currentWindowBoundsNew);
        }
    }

    void WindowGeometryHelper::CenterWindowOnMonitor(
        BackendApi::IBackendWindowHelper* backendWindowHelper, BackendApi::WindowPointer window,
        std::function<void()> fnBeforeChangingWindowBounds)
    {
        ScreenBounds windowBounds = backendWindowHelper->GetWindowBounds(window);
        ScreenBounds currentMonitorWorkArea = GetCurrentMonitorWorkArea(backendWindowHelper, window);
        ScreenPosition newWindowPosition = currentMonitorWorkArea.WinPositionCentered(windowBounds.size);
        windowBounds.position = newWindowPosition;
        fnBeforeChangingWindowBounds();
        backendWindowHelper->SetWindowBounds(window, windowBounds);
    }
}
