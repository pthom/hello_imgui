#ifdef HELLOIMGUI_USE_GLFW
#include "glfw_window_helper.h"


namespace HelloImGui { namespace BackendApi
{
    static void glfw_error_callback(int error, const char* description)
    {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    }


    WindowPointer GlfwWindowHelper::CreateWindow(AppWindowParams &info, const BackendOptions& backendOptions)
    {
        GLFWwindow *noWindowSharedResources = nullptr;
        GLFWmonitor *monitor = nullptr;

        int nbMonitors;
        auto monitors = glfwGetMonitors(&nbMonitors);
        int monitorIdx = info.windowGeometry.monitorIdx;
        assert((monitorIdx >= 0) && (monitorIdx < nbMonitors));

        auto fullScreenMode = info.windowGeometry.fullScreenMode;
        auto &windowSize = info.windowGeometry.size;
        ScreenPosition & windowPosition = info.windowGeometry.position;

        if (fullScreenMode == FullScreenMode::FullMonitorWorkArea)
        {
            auto monitorBounds = GetOneMonitorWorkArea(info.windowGeometry.monitorIdx);
            windowSize = monitorBounds.size;
            info.windowGeometry.position = monitorBounds.position;
        } else if (fullScreenMode == FullScreenMode::FullScreenDesktopResolution)
        {
            monitor = monitors[monitorIdx];

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
            monitor = monitors[monitorIdx];
        } else if (fullScreenMode == FullScreenMode::NoFullScreen)
            {}
        else
        {
            BACKEND_THROW("Unexpected fullScreenMode");
        }

        // info.backend3DMode: not handled here

        // info.allowHighDpi: not handled

        if (info.borderless)
            glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        else
            glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

        if (info.resizable)
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        else
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        auto window = glfwCreateWindow(
            windowSize[0], windowSize[1],
            info.windowTitle.c_str(),
            monitor,  // monitor
            noWindowSharedResources
        );
        if (window == nullptr)
            BACKEND_THROW("BackendGlfw::CreateWindow / glfwCreateWindow failed");

        if (info.windowSizeState == WindowSizeState::Minimized)
            glfwIconifyWindow(window);
        else if (info.windowSizeState == WindowSizeState::Maximized)
            glfwMaximizeWindow(window);


        WindowPositionMode positionMode = info.windowGeometry.positionMode;
        if ( (positionMode == WindowPositionMode::FromCoords) || (fullScreenMode == FullScreenMode::FullMonitorWorkArea))
        {
            glfwSetWindowPos(window, windowPosition[0], windowPosition[1]);
            // We need to set the size again, in case we changed monitor
            glfwSetWindowSize(window, windowSize[0], windowSize[1]);
        }
        else if ( (positionMode == WindowPositionMode::MonitorCenter) && (fullScreenMode==FullScreenMode::NoFullScreen))
        {
            auto workArea = GetOneMonitorWorkArea(monitorIdx);
            ScreenPosition centeredPosition;
            ForDim2(dim)
                centeredPosition[dim] = workArea.Center()[dim] - windowSize[dim] / 2;
            glfwSetWindowPos(window, centeredPosition[0], centeredPosition[1]);
            // We need to set the size again, in case we changed monitor
            glfwSetWindowSize(window, windowSize[0], windowSize[1]);
        }

        glfwGetWindowSize(window, &windowSize[0], &windowSize[1]);
        glfwGetWindowPos(window, &windowPosition[0], &windowPosition[1]);

        printf("Final window size: %ix%i\n", windowSize[0], windowSize[1]);
        printf("Final window position: %ix%i\n", windowPosition[0], windowPosition[1]);

        return (void *)(window);
    }

    size_t GlfwWindowHelper::GetNbMonitors()
    {
        int nbMonitors;
        auto monitors = glfwGetMonitors(&nbMonitors);
        (void) monitors;
        return nbMonitors;
    }

    ScreenBounds GlfwWindowHelper::GetOneMonitorWorkArea(int monitorIndex)
    {
        int nbMonitors;
        auto monitors = glfwGetMonitors(&nbMonitors);
        assert((monitorIndex >= 0) && (monitorIndex < nbMonitors));
        int x, y, w, h;
        glfwGetMonitorWorkarea(monitors[monitorIndex], &x, &y, &w, &h);
        auto r = ScreenBounds{{x, y},
                              {w, h}};
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
}} // namespace HelloImGui { namespace BackendApi
#endif // #ifdef HELLOIMGUI_USE_GLFW