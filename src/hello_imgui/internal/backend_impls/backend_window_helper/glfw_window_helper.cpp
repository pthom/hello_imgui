#ifdef HELLOIMGUI_USE_GLFW
#include "glfw_window_helper.h"
#include "window_geometry_helper.h"
#include "GLFW/glfw3.h"


namespace HelloImGui { namespace BackendApi
{
    static void glfw_error_callback(int error, const char* description)
    {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    }

    WindowPointer GlfwWindowHelper::CreateWindow(AppWindowParams &appWindowParams, const BackendOptions& backendOptions)
    {
        auto searchMonitorResult = SearchForMonitor(GetMonitorsWorkAreas(), appWindowParams);
        int realMonitorIdx = searchMonitorResult.monitorIdx;
        if (searchMonitorResult.newPosition.has_value())
            appWindowParams.windowGeometry.position = searchMonitorResult.newPosition.value();


        GLFWwindow *noWindowSharedResources = nullptr;
        GLFWmonitor *monitor = nullptr;

        auto fullScreenMode = appWindowParams.windowGeometry.fullScreenMode;
        auto &windowSize = appWindowParams.windowGeometry.size;
        ScreenPosition & windowPosition = appWindowParams.windowGeometry.position;

        auto monitorsWorkAreas = GetMonitorsWorkAreas();

        // Reduce size if too big compared to the monitor
        if (! appWindowParams.windowGeometry.sizeAuto)
        {
            auto workArea = monitorsWorkAreas[realMonitorIdx];
            ForDim2(dim)
                if (windowSize[dim] > workArea.size[dim])
                    windowSize[dim] = workArea.size[dim];
        }

        if (fullScreenMode == FullScreenMode::FullMonitorWorkArea)
        {
            auto monitorBounds = monitorsWorkAreas[appWindowParams.windowGeometry.monitorIdx];
            windowSize = monitorBounds.size;
            appWindowParams.windowGeometry.position = monitorBounds.position;
        }
        else if (fullScreenMode == FullScreenMode::FullScreenDesktopResolution)
        {
            int nbMonitors;
            auto monitors = glfwGetMonitors(&nbMonitors);
            monitor = monitors[realMonitorIdx];

            const GLFWvidmode *mode = glfwGetVideoMode(monitor);
            glfwWindowHint(GLFW_RED_BITS, mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
            windowSize[0] = mode->width;
            windowSize[1] = mode->height;
        }
        else if (fullScreenMode == FullScreenMode::FullScreen)
        {
            int nbMonitors;
            auto monitors = glfwGetMonitors(&nbMonitors);
            monitor = monitors[realMonitorIdx];
        } else if (fullScreenMode == FullScreenMode::NoFullScreen)
            {}
        else
        {
            BACKEND_THROW("Unexpected fullScreenMode");
        }

        // info.backend3DMode: not handled here

        // info.allowHighDpi: not handled

        if (appWindowParams.borderless)
            glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        else
            glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

        if (appWindowParams.resizable)
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        else
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);


        // When high dpi, make the window bigger at creation, so that window sizes are consistent across
        // platforms after creation
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);


        auto window = glfwCreateWindow(
            windowSize[0], windowSize[1],
            appWindowParams.windowTitle.c_str(),
            monitor,  // monitor
            noWindowSharedResources
        );
        if (window == nullptr)
            BACKEND_THROW("BackendGlfw::CreateWindow / glfwCreateWindow failed");

        if (appWindowParams.windowGeometry.windowSizeState == WindowSizeState::Minimized)
            glfwIconifyWindow(window);
        else if (appWindowParams.windowGeometry.windowSizeState == WindowSizeState::Maximized)
            glfwMaximizeWindow(window);


        WindowPositionMode positionMode = appWindowParams.windowGeometry.positionMode;
        if ( (positionMode == WindowPositionMode::FromCoords) || (fullScreenMode == FullScreenMode::FullMonitorWorkArea))
        {
            glfwSetWindowPos(window, windowPosition[0], windowPosition[1]);
            // We need to set the size again, in case we changed monitor
            glfwSetWindowSize(window, windowSize[0], windowSize[1]);
        }
        else if ( (positionMode == WindowPositionMode::MonitorCenter) && (fullScreenMode==FullScreenMode::NoFullScreen))
        {
            auto workArea = monitorsWorkAreas[realMonitorIdx];
            ScreenPosition centeredPosition;
            ForDim2(dim)
                centeredPosition[dim] = workArea.Center()[dim] - windowSize[dim] / 2;
            glfwSetWindowPos(window, centeredPosition[0], centeredPosition[1]);
            // We need to set the size again, in case we changed monitor
            glfwSetWindowSize(window, windowSize[0], windowSize[1]);
        }

        glfwGetWindowSize(window, &windowSize[0], &windowSize[1]);
        glfwGetWindowPos(window, &windowPosition[0], &windowPosition[1]);

        // printf("Final window size: %ix%i\n", windowSize[0], windowSize[1]);
        // printf("Final window position: %ix%i\n", windowPosition[0], windowPosition[1]);

        return (void *)(window);
    }

    std::vector<ScreenBounds> GlfwWindowHelper::GetMonitorsWorkAreas()
    {
        int nbMonitors;
        auto monitors = glfwGetMonitors(&nbMonitors);
        std::vector<ScreenBounds> r;
        for (int i =0; i < nbMonitors; ++i)
        {
            int x, y, w, h;
            glfwGetMonitorWorkarea(monitors[i], &x, &y, &w, &h);
            auto b = ScreenBounds{{x, y},
                                  {w, h}};
            r.push_back(b);
        }
        return r;
    }

    bool GlfwWindowHelper::IsWindowIconified(WindowPointer window)
    {
        auto glfwWindow = (GLFWwindow *)(window);
        bool iconified = glfwGetWindowAttrib(glfwWindow, GLFW_ICONIFIED) != 0;
        bool hidden = (glfwGetWindowAttrib(glfwWindow, GLFW_VISIBLE) == 0);
        return (iconified || hidden);
    }

    void GlfwWindowHelper::RaiseWindow(WindowPointer window)
    {
        auto glfwWindow = (GLFWwindow *)(window);
        glfwShowWindow(glfwWindow);
        glfwFocusWindow(glfwWindow);
        glfwRequestWindowAttention(glfwWindow);
    }

    ScreenBounds GlfwWindowHelper::GetWindowBounds(WindowPointer window)
    {
        ScreenBounds windowBounds;
        auto glfwWindow = (GLFWwindow *)(window);
        glfwGetWindowPos(glfwWindow, &windowBounds.position[0], &windowBounds.position[1]);
        glfwGetWindowSize(glfwWindow, &windowBounds.size[0], &windowBounds.size[1]);
        return windowBounds;
    }

    void GlfwWindowHelper::SetWindowBounds(WindowPointer window, ScreenBounds windowBounds)
    {
        auto glfwWindow = (GLFWwindow *)(window);
        glfwSetWindowPos(glfwWindow, windowBounds.position[0], windowBounds.position[1]);
        glfwSetWindowSize(glfwWindow, windowBounds.size[0], windowBounds.size[1]);
    }

    void GlfwWindowHelper::WaitForEventTimeout(double timeout_seconds)
    {
        glfwWaitEventsTimeout(timeout_seconds);
    }

    float GlfwWindowHelper::GetWindowDpiScaleFactor(WindowPointer window)
    {
#ifdef __APPLE__
        return 1.f;
#else
        float xscale, yscale;
        glfwGetWindowContentScale((GLFWwindow *) window, &xscale, &yscale);
        if (xscale > yscale)
            return xscale;
        else
            return yscale;
#endif
    }
}} // namespace HelloImGui { namespace BackendApi
#endif // #ifdef HELLOIMGUI_USE_GLFW