#ifdef HELLOIMGUI_USE_GLFW3
#include "glfw_window_helper.h"
#include "window_geometry_helper.h"
#include "GLFW/glfw3.h"


namespace HelloImGui { namespace BackendApi
{
    std::function<void()> gRenderCallbackDuringResize_Glfw;

    static void FramebufferSizeCallback(GLFWwindow*, int width, int height)
    {
        // Not used, since WindowSizeCallback is called right after.
    }
    static void WindowSizeCallback(GLFWwindow* window, int width, int height)
    {
        // See https://github.com/pthom/hello_imgui/issues/112
        // This may trigger a reentrant call to
        //    AbstractRunner::CreateFramesAndRender()
        // By default, this is disabled.
        // See pref AppWindowParams.repaintDuringResize_GotchaReentrantRepaint
        if (gRenderCallbackDuringResize_Glfw)
            gRenderCallbackDuringResize_Glfw();
    }



    WindowPointer GlfwWindowHelper::CreateWindow(AppWindowParams &appWindowParams, const BackendOptions& backendOptions,
                                                 std::function<void()> renderCallbackDuringResize)
    {
        gRenderCallbackDuringResize_Glfw = renderCallbackDuringResize;

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

        // Note: This is RenderingCallbacks_Impl_Hint_WindowingBackend
        if (backendOptions.rendererBackendType != RendererBackendType::OpenGL3)
        {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        if (appWindowParams.borderless)
            glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        else
            glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

        if (appWindowParams.resizable)
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        else
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        // GLFW_SCALE_TO_MONITOR is set to false: we set manually the window size in screen coordinates,
        // then AbstractRunner::MakeWindowSizeRelativeTo96Ppi_IfRequired() may resize the window at the second frame
        // (depending on the monitor on which it is placed)
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);

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

        glfwSetWindowSizeCallback(window, WindowSizeCallback);
        glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
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
            auto bounds = ScreenBounds{{x, y},
                                  {w, h}};
#ifdef _WIN32
            // Hack because glfw and sdl behave differently on window position
            bounds.position[1] += 12; 
            bounds.size[1] -= 12;
#endif
            r.push_back(bounds);
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

    ImVec2 _GetWindowContentScale(HelloImGui::BackendApi::WindowPointer window)
    {
        float x_scale, y_scale;
        glfwGetWindowContentScale((GLFWwindow *) window, &x_scale, &y_scale);
        return ImVec2(x_scale, y_scale);
    }

    float GlfwWindowHelper::GetWindowSizeDpiScaleFactor(WindowPointer window)
    {
#ifdef __APPLE__
        return 1.f;
#else
        ImVec2 scale = _GetWindowContentScale(window);
        return scale.x;
#endif
    }

    void GlfwWindowHelper::HideWindow(WindowPointer window)
    {
        glfwHideWindow((GLFWwindow *) window);
    }
    void GlfwWindowHelper::ShowWindow(WindowPointer window)
    {
        glfwShowWindow((GLFWwindow *) window);
    }
    bool GlfwWindowHelper::IsWindowHidden(WindowPointer window)
    {
        int visible = glfwGetWindowAttrib((GLFWwindow *) window, GLFW_VISIBLE);
        return (visible == 0);
    }

//    void truc(WindowPointer window)
//    {
//        glfwCur
//
//    }

}} // namespace HelloImGui { namespace BackendApi
#endif // #ifdef HELLOIMGUI_USE_GLFW3