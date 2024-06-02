#pragma once
#include "hello_imgui/app_window_params.h"
#include "hello_imgui/runner_params.h"

#include <string>
#include <iostream>
#include <optional>
#include <vector>

#ifdef _WIN32
#ifdef CreateWindow
#undef CreateWindow
#endif
#endif

#define BACKEND_THROW_IMPL(msg) \
    { \
        std::cerr << "throw runtime_error: " << msg << "\t\t at " << __FILE__ << ":" << __LINE__ << "\n"; \
        throw std::runtime_error(msg); \
    }

#define BACKEND_THROW(...) BACKEND_THROW_IMPL((__VA_ARGS__));


namespace HelloImGui { namespace BackendApi
{
    struct BackendOptions
    {
        RendererBackendType rendererBackendType = RendererBackendType::OpenGL3;
        // this will enable dpi awareness under windows, and set the window flag SDL_WINDOW_ALLOW_HIGHDPI
        // (note: glfw is dpi aware by default)
        bool dpiAwareSdl = true; 
    };


    // Container for pointers to SDLWindow, GLFWwindow, etc.
    using WindowPointer = void *;


    class IBackendWindowHelper
    {
        // Note: this is a fake class, it has no member
        // It is only a class in order to enforce a consistent API between backends.
    public:
        virtual ~IBackendWindowHelper() = default;

        // renderCallbackDuringResize is a callback that will be called during window resizing.
        // to trigger a render
        // This is due to severe gotcha inside GLFW and SDL: PollEvent is supposed to
        // return immediately, but it doesn't when resizing the window!
        // Instead, you have to subscribe to a kind of special "mid-resize" event,
        // and then call the render function yourself.
        virtual WindowPointer CreateWindow(AppWindowParams &info, const BackendOptions& backendOptions,
                                           std::function<void()> renderCallbackDuringResize) = 0;

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

        // (ImGui backends handle this by themselves)
        //virtual ImVec2 GetDisplayFramebufferScale(WindowPointer window) = 0;

        // Return the ratio by which the window size should be scaled to account for HighDPI
        // (i.e. the same size given at creation create the same physical size in mm on the screen)
        virtual float GetWindowSizeDpiScaleFactor(WindowPointer window) = 0;

        virtual void HideWindow(WindowPointer window) = 0;
        virtual void ShowWindow(WindowPointer window) = 0;
        virtual bool IsWindowHidden(WindowPointer window) = 0;

        // Screenshot!
    };
}} // namespace HelloImGui { namespace BackendApi
