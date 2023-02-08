#pragma once
#include "hello_imgui/app_window_params.h"

#include <string>
#include <iostream>
#include <optional>
#include <vector>


#define BACKEND_THROW_IMPL(msg) \
    { \
        std::cerr << "throw runtime_error: " << msg << "\t\t at " << __FILE__ << ":" << __LINE__ << "\n"; \
        throw std::runtime_error(msg); \
    }

#define BACKEND_THROW(...) BACKEND_THROW_IMPL((__VA_ARGS__));


namespace HelloImGui { namespace BackendApi
{
    enum class Backend3dMode
    {
        No3d,
        OpenGl,
        Vulkan,
        Metal,
        DirectX
    };


    struct BackendOptions
    {
        // this will enable dpi awareness under windows, and set the window flag SDL_WINDOW_ALLOW_HIGHDPI
        // (note: glfw is dpi aware by default)
        bool dpiAwareSdl = true; 

        Backend3dMode backend3DMode = Backend3dMode::OpenGl;
    };


    // Container for pointers to SDLWindow, GLFWwindow, etc.
    using WindowPointer = void *;


    class IBackendWindowHelper
    {
        // Note: this is a fake class, it has no member
        // It is only a class in order to enforce a consistent API between backends.
    public:
        virtual ~IBackendWindowHelper() = default;
        virtual WindowPointer CreateWindow(AppWindowParams &info, const BackendOptions& backendOptions) = 0;

        // The area of a monitor not occupied by global task bars or menu bars is the work area.
        // This is specified in screen coordinates.
        // Pixels and screen coordinates may map 1:1 on your machine, but they won't on every other machine,
        // for example on a Mac with a Retina display.
        virtual std::vector<ScreenBounds> GetMonitorsWorkAreas() = 0;

        virtual bool IsWindowIconified(WindowPointer window) = 0;

        virtual void RaiseWindow(WindowPointer window) = 0;

        virtual ScreenBounds GetWindowBounds(WindowPointer window) = 0;

        virtual void SetWindowBounds(WindowPointer window, ScreenBounds windowBounds) = 0;

        virtual void WaitForEventTimeout(double timeout_seconds) = 0;

        virtual float GetWindowSizeDpiScaleFactor(WindowPointer window) = 0;

        virtual void HideWindow(WindowPointer window) = 0;
        virtual void ShowWindow(WindowPointer window) = 0;
        virtual bool IsWindowHidden(WindowPointer window) = 0;

        // Screenshot!
    };
}} // namespace HelloImGui { namespace BackendApi
