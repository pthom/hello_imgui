#ifdef HELLOIMGUI_USE_GLFW
#include "glfw_window_helper.h"


namespace BackendApi
{
    static void glfw_error_callback(int error, const char* description)
    {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    }


    WindowPointer GlfwWindowHelper::CreateWindow(WindowOptions &info)
    {
        GLFWwindow *noWindowSharedResources = nullptr;
        GLFWmonitor *monitor = nullptr;

        ScreenBounds &windowBounds = info.windowBounds;
        if (info.fullScreenMode == FullScreenMode::FullMonitorWorkArea)
        {
            auto monitorBounds = GetOneMonitorWorkArea(info.monitorIdx);
            windowBounds = monitorBounds;
        } else if (info.fullScreenMode == FullScreenMode::FullScreenDesktopResolution)
        {
            int nbMonitors;
            auto monitors = glfwGetMonitors(&nbMonitors);
            assert((info.monitorIdx > 0) && (info.monitorIdx < nbMonitors));
            monitor = monitors[info.monitorIdx];

            const GLFWvidmode *mode = glfwGetVideoMode(monitor);
            glfwWindowHint(GLFW_RED_BITS, mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
            windowBounds.size[0] = mode->width;
            windowBounds.size[1] = mode->height;
        }
        else if (info.fullScreenMode == FullScreenMode::FullScreen)
        {
            int nbMonitors;
            auto monitors = glfwGetMonitors(&nbMonitors);
            assert((info.monitorIdx > 0) && (info.monitorIdx < nbMonitors));
            monitor = monitors[info.monitorIdx];
        } else if (info.fullScreenMode == FullScreenMode::NoFullScreen)
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
            windowBounds.size[0], windowBounds.size[1],
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
} // namespace BackendApi
#endif // #ifdef HELLOIMGUI_USE_GLFW