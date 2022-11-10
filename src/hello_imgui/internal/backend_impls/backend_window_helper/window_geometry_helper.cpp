#include "window_geometry_helper.h"

#include <filesystem>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>


namespace
{
    const char* APP_WINDOW_POS_INI_FILE = "imgui_app_window.ini";

    std::vector<std::string> splitString(const std::string& s, char delimiter)
    {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter))
        {
            tokens.push_back(token);
        }
        return tokens;
    }

    bool stringStartsWith(const std::string& s, const std::string& prefix)
    {
        if (s.size() < prefix.size())
            return false;
        auto start = s.substr(0, prefix.size());
        return start == prefix;
    }

    std::optional<std::string> stringAfter(const std::string& s, const std::string& prefix)
    {
        if (!stringStartsWith(s, prefix))
            return std::nullopt;
        auto end = s.substr(prefix.size());
        return end;
    }

    std::optional<std::array<int, 2>> parseSizeLine(const std::string& line, const std::string& itemName)
    {
        auto values = stringAfter(line, itemName + "=");
        if (!values.has_value())
            return std::nullopt;
        std::vector<std::string> items = splitString(values.value(), ',');
        if (items.size() != 2)
            return std::nullopt;

        std::array<int, 2> r;
        for (size_t i = 0; i < 2; ++i)
        {
            try
            {
                r[i] = std::stoi(items[i]);
            }
            catch(std::exception&)
            {
                return std::nullopt;
            }
        }
        return r;
    }
}

namespace HelloImGui
{
    WindowGeometryHelper::WindowGeometryHelper(WindowGeometry &geometry, bool restoreLast) :
        mGeometry(geometry), mRestoreLast(restoreLast)
        {}

    bool WindowGeometryHelper::HasWindowSizeInfo()
    {
        if (
            (mGeometry.windowSizeState == WindowSizeState::Maximized)
            || (mGeometry.fullScreenMode != FullScreenMode::NoFullScreen))
        {
            return true;
        }
        if (!mGeometry.sizeAuto)
            return true;
        if (mRestoreLast && WindowGeometryHelper::ReadLastRunWindowBounds().has_value())
            return true;

        return false;
    }

    ScreenBounds WindowGeometryHelper::AppWindowBoundsInitial(
        const std::vector<ScreenBounds>& allMonitorsWorkAreas,
        const std::optional<ScreenSize>& realWindowSizeAfterAutoSize
    )
    {
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
          windowBoundsLastRun = WindowGeometryHelper::ReadLastRunWindowBounds();

        // Window Size
        auto computeSize = [&]() -> ScreenSize
        {
            if (realWindowSizeAfterAutoSize.has_value())
                return realWindowSizeAfterAutoSize.value();
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
            assert((monitorIdx >= 0) && (monitorIdx < allMonitorsWorkAreas.size()));
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


    void WindowGeometryHelper::WriteLastRunWindowBounds(const ScreenBounds& windowBounds)
    {
        std::stringstream ss;
        ss << "[WIN]\n";
        ss << "WindowPosition=" << windowBounds.position[0] << "," << windowBounds.position[1] << "\n";
        ss << "WindowSize=" << windowBounds.size[0] << "," << windowBounds.size[1] << "\n";

        std::ofstream os(APP_WINDOW_POS_INI_FILE);
        os << ss.str();
        os.close();

#ifdef _DEBUG_IMGUI_RUNNER
        std::cout << ss.str();
#endif
    }

    std::optional<ScreenBounds> WindowGeometryHelper::ReadLastRunWindowBounds()
    {
        std::ifstream is(APP_WINDOW_POS_INI_FILE);
        if (is.is_open())
        {
            ScreenBounds r;
            std::string line;
            while (is)
            {
                std::getline(is, line);

                auto pos = parseSizeLine(line, "WindowPosition");
                if (pos.has_value())
                    r.position = pos.value();

                auto size = parseSizeLine(line, "WindowSize");
                if (size.has_value())
                    r.size = size.value();
            }

            return r;
        }
        else
            return std::nullopt;
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
            assert(monitorsWorkAreas.size() > 0);
            auto& wantedPosition = geometry.position;
            for (size_t monitorIdx = 0; monitorIdx < monitorsWorkAreas.size(); ++monitorIdx)
            {
                auto workArea = monitorsWorkAreas[monitorIdx];
                if (workArea.Contains(wantedPosition))
                    foundMonitorIdx = monitorIdx;
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


}