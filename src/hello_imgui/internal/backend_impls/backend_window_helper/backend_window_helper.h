#pragma once
#include "screen_bounds.h"

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

        Backend3dMode backend3DMode = Backend3dMode::OpenGl;

        bool borderless = false;
        bool resizable = false;

        WindowVisibility windowVisibility = WindowVisibility::Standard;
    };


    // Container for pointers to SDLWindow, GLFWwindow, etc.
    using WindowPointer = void *;


    class IBackendWindowHelper
    {
        // Note: this is a fake class, it has no member
        // It is only a class in order to enforce a consistent API between backends.
    public:
        virtual WindowPointer CreateWindow(WindowOptions &info) = 0;

        virtual size_t GetNbMonitors() = 0;

        //The area of a monitor not occupied by global task bars or menu bars is the work area.
        //This is specified in screen coordinates.
        //Pixels and screen coordinates may map 1:1 on your machine, but they won't on every other machine,
        //for example on a Mac with a Retina display.
        virtual ScreenBounds GetOneMonitorWorkArea(int monitorIndex) = 0;

        virtual bool IsWindowIconified(WindowPointer window) = 0;

        virtual void RaiseWindow(WindowPointer window) = 0;

        virtual ScreenBounds GetWindowBounds(WindowPointer window) = 0;

        virtual void SetWindowBounds(WindowPointer window, ScreenBounds windowBounds) = 0;

        // Screenshot!

    };
}