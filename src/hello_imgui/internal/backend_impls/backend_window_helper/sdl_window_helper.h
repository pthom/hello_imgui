#pragma once
#ifdef HELLOIMGUI_USE_SDL2

#include "hello_imgui/internal/backend_impls/backend_window_helper/backend_window_helper.h"

#include <cmath>
#include <string>
#include <memory>
#include <iostream>


#include "SDL.h"
#include "SDL_main.h"

#ifdef _WIN32
#ifdef CreateWindow
#undef CreateWindow
#endif
#endif

namespace HelloImGui { namespace BackendApi
{
    class SdlWindowHelper : public IBackendWindowHelper
    {
        // Note: this is a fake class, it has no member
        // It is only a class in order to enforce a consistent API between backends.
    public:
        WindowPointer CreateWindow(AppWindowParams &appWindowParams, const BackendOptions& backendOptions,
                                   std::function<void()> renderCallbackDuringResize) override;

        std::vector<ScreenBounds> GetMonitorsWorkAreas() override;

        bool IsWindowIconified(WindowPointer window) override;

        void RaiseWindow(WindowPointer window) override;

        ScreenBounds GetWindowBounds(WindowPointer window) override;
        void SetWindowBounds(WindowPointer window, ScreenBounds windowBounds) override;

        void WaitForEventTimeout(double timeout_seconds) override;

        float GetWindowSizeDpiScaleFactor(WindowPointer window) override;

        void HideWindow(WindowPointer window) override;
        void ShowWindow(WindowPointer window) override;
        bool IsWindowHidden(WindowPointer window) override;
    };
}} // namespace HelloImGui { namespace BackendApi

#endif // #ifdef HELLOIMGUI_USE_SDL2
