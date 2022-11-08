#pragma once
#ifdef HELLOIMGUI_USE_SDL

#include "internal/backend_impls/backend_window_helper/backend_window_helper.h"

#include <cmath>
#include <string>
#include <memory>
#include <iostream>


#include "SDL.h"
#include "SDL_main.h"


namespace HelloImGui { namespace BackendApi
{
    class SdlWindowHelper : public IBackendWindowHelper
    {
        // Note: this is a fake class, it has no member
        // It is only a class in order to enforce a consistent API between backends.
    public:
        WindowPointer CreateWindow(AppWindowParams &appWindowParams, const BackendOptions& backendOptions) override;

        std::vector<ScreenBounds> GetMonitorsWorkAreas() override;

        bool IsWindowIconified(WindowPointer window) override;

        void RaiseWindow(WindowPointer window) override;

        ScreenBounds GetWindowBounds(WindowPointer window) override;
        void SetWindowBounds(WindowPointer window, ScreenBounds windowBounds) override;

        void WaitForEventTimeout(double timeout_seconds) override;

        virtual float GetWindowDpiScaleFactor(WindowPointer window) override;
    };
}} // namespace HelloImGui { namespace BackendApi

#endif // #ifdef HELLOIMGUI_USE_SDL
