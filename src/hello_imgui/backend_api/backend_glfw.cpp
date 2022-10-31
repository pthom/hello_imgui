#include "hello_imgui/backend_api/backend_glfw.h"


namespace BackendApi
{
    BackendGlfwWindow* BackendGlfw::GetBackendGlfwWindow(IBackendWindow *bw)
    {
        BackendGlfwWindow *backendGlfwWindow = dynamic_cast<BackendGlfwWindow *>(bw);
        return backendGlfwWindow;
    }

    GLFWwindow* BackendGlfw::GetGlfwWindow(IBackendWindow *bw)
    {
        BackendGlfwWindow *backendGlfwWindow = dynamic_cast<BackendGlfwWindow *>(bw);
        return backendGlfwWindow->mWindow;
    }

    void BackendGlfw::Init()
    {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
        BACKEND_THROW("BackendGlfw::Init glfwInit failed");
    }

    void BackendGlfw::DeInit()
    {
        glfwTerminate();
    }

    IBackendWindow* BackendGlfw::CreateWindow(WindowOptions &info)
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

        if (info.windowVisibility == WindowVisibility::Minimized)
            glfwIconifyWindow(window);
        else if (info.windowVisibility == WindowVisibility::Maximized)
            glfwMaximizeWindow(window);

        auto r = new BackendGlfwWindow(window);
        return r;
    }

    void BackendGlfw::DestroyWindow(IBackendWindow *window)
    {
        auto glfwWindow = GetGlfwWindow(window);
        glfwDestroyWindow(glfwWindow);
        delete (window);
    }

    size_t BackendGlfw::GetNbMonitors()
    {
        int nbMonitors;
        auto monitors = glfwGetMonitors(&nbMonitors);
        (void) monitors;
        return nbMonitors;
    }

    ScreenBounds BackendGlfw::GetOneMonitorWorkArea(int monitorIndex)
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

    bool BackendGlfw::IsWindowIconified(IBackendWindow *window)
    {
        auto glfwWindow = GetGlfwWindow(window);
        bool iconified = glfwGetWindowAttrib(glfwWindow, GLFW_ICONIFIED) != 0;
        bool hidden = (glfwGetWindowAttrib(glfwWindow, GLFW_VISIBLE) == 0);
        return (iconified || hidden);
    }

    bool BackendGlfw::ShouldWindowClose(IBackendWindow *window)
    {
        auto glfwWindow = GetGlfwWindow(window);
        bool shouldClose = (glfwWindowShouldClose(glfwWindow) != 0);
        return shouldClose;
    }

    void BackendGlfw::RaiseWindow(IBackendWindow *window)
    {
        auto glfwWindow = GetGlfwWindow(window);
        glfwShowWindow(glfwWindow);
        glfwFocusWindow(glfwWindow);
        glfwRequestWindowAttention(glfwWindow);
    }

    ScreenBounds BackendGlfw::GetWindowBounds(IBackendWindow *window)
    {
        ScreenBounds windowBounds;
        auto glfwWindow = GetGlfwWindow(window);
        glfwGetWindowPos(glfwWindow, &windowBounds.position[0], &windowBounds.position[1]);
        glfwGetWindowSize(glfwWindow, &windowBounds.size[0], &windowBounds.size[1]);
        return windowBounds;
    }

    void BackendGlfw::SetWindowBounds(IBackendWindow *window, ScreenBounds windowBounds)
    {
        auto glfwWindow = GetGlfwWindow(window);
        glfwSetWindowPos(glfwWindow, windowBounds.position[0], windowBounds.position[1]);
        glfwSetWindowSize(glfwWindow, windowBounds.size[0], windowBounds.size[1]);
    }

    void BackendGlfw::WaitForEvent(IBackendWindow *window, int timeOutMilliseconds)
    {
        if (timeOutMilliseconds > 0)
            glfwWaitEventsTimeout(timeOutMilliseconds);
    }

    void BackendGlfw::PollEvents(IBackendWindow *window, const AnyEventCallback &anyEventCallback)
    {
        glfwPollEvents();
    }

} // namespace BackendApi
