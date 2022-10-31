#pragma once
#include "hello_imgui/backend_api/screen_bounds.h"

#include <string>
#include <iostream>


#define BACKEND_THROW_IMPL(msg) \
    { \
        std::cerr << "throw runtime_error: " << msg << "\t\t at " << __FILE__ << ":" << __LINE__ << "\n"; \
        throw std::runtime_error(msg); \
    }

#define BACKEND_THROW(...) BACKEND_THROW_IMPL((__VA_ARGS__));


namespace BackendApi
{
    enum class FullScreenMode
    {
        NoFullScreen,
        FullScreen,                    // Full screen with specified resolution
        FullScreenDesktopResolution,   // Full screen with current desktop mode & resolution
        FullMonitorWorkArea            // Fake full screen, maximized window on the selected monitor
    };


    enum class Backend3dMode
    {
        No3d,
        OpenGl,
        Vulkan,
        Metal,
        DirectX
    };


    enum class WindowVisibility
    {
        Standard,
        Minimized,
        Maximized
    };


    struct WindowOptions
    {
        std::string windowTitle = "";

        // If windowBound.windowPosition
        ScreenBounds windowBounds = {};

        FullScreenMode fullScreenMode = FullScreenMode::NoFullScreen;
        int monitorIdx = 0; // Will be used in full screen mode, or if windowBounds.Position is unspecified

        // If the window is created with the `allowHighDpi` flag, its size
        // * in pixels may differ from its size in screen coordinates on platforms with
        // * high-DPI support (e.g. iOS and macOS)
        bool allowHighDpi = true;

        Backend3dMode backend3DMode;

        bool borderless = false;
        bool resizable = false;

        WindowVisibility windowVisibility;
    };


/**
**AnyEventCallback** can hold any bool(void *) function.
It is designed to handle callbacks for a specific backend.
````cpp
using AnyEventCallback = std::function<bool(void * backendEvent)>
````
**/
    using AnyEventCallback = std::function<bool(void *backendEvent)>;

    bool EventCallback_NoOp(void *backendEvent);


    // Container for pointers to SDLWindow, GLFWwindow, etc.
    class IBackendWindow
    {
    public:
        virtual ~IBackendWindow()
        {}
    };


    class IBackend
    {
    public:
        virtual void Init() = 0;

        virtual void DeInit() = 0;

        virtual IBackendWindow *CreateWindow(WindowOptions &info) = 0;

        virtual void DestroyWindow(IBackendWindow *window) = 0;

        virtual size_t GetNbMonitors() = 0;

        //The area of a monitor not occupied by global task bars or menu bars is the work area.
        //This is specified in screen coordinates.
        //Pixels and screen coordinates may map 1:1 on your machine, but they won't on every other machine,
        //for example on a Mac with a Retina display.
        virtual ScreenBounds GetOneMonitorWorkArea(int monitorIndex) = 0;

        virtual bool IsWindowIconified(IBackendWindow *window) = 0;

        virtual bool ShouldWindowClose(IBackendWindow *window) = 0;

        virtual void RaiseWindow(IBackendWindow *window) = 0;

        virtual ScreenBounds GetWindowBounds(IBackendWindow *window) = 0;

        virtual void SetWindowBounds(IBackendWindow *window, ScreenBounds windowBounds) = 0;

        virtual void WaitForEvent(IBackendWindow *window, int timeOutMilliseconds) = 0;

        virtual void
        PollEvents(IBackendWindow *window, const AnyEventCallback &anyEventCallback = EventCallback_NoOp) = 0;

        // Screenshot!

    };
}