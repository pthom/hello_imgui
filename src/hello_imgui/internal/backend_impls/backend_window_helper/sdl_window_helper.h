#pragma once
#ifdef HELLOIMGUI_USE_SDL

#include "internal/backend_impls/backend_window_helper/backend_window_helper.h"

#include <cmath>
#include <string>
#include <memory>
#include <iostream>


#include "SDL.h"
#include "SDL_main.h"


namespace BackendApi
{
    class SdlWindowHelper : public IBackendWindowHelper
    {
        // Note: this is a fake class, it has no member
        // It is only a class in order to enforce a consistent API between backends.
    public:
        WindowPointer CreateWindow(WindowOptions &info) override;

        size_t GetNbMonitors() override;
        ScreenBounds GetOneMonitorWorkArea(int monitorIndex) override;

        bool IsWindowIconified(WindowPointer window) override;

        void RaiseWindow(WindowPointer window) override;

        ScreenBounds GetWindowBounds(WindowPointer window) override;
        void SetWindowBounds(WindowPointer window, ScreenBounds windowBounds) override;
    };
} // namespace BackendApi

#endif // #ifdef HELLOIMGUI_USE_SDL
