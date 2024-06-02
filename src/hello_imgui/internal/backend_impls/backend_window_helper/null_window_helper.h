#pragma once

#include "backend_window_helper.h"
#include "hello_imgui/internal/backend_impls/null_config.h"
#include <thread>

#ifdef _WIN32
#ifdef CreateWindow
#undef CreateWindow
#endif
#endif


namespace HelloImGui { namespace BackendApi
{
    class NullWindowHelper: public IBackendWindowHelper
    {
        // Note: this is a fake class, it has no member
        // It is only a class in order to enforce a consistent API between backends.
    public:
        WindowPointer CreateWindow(AppWindowParams &appWindowParams, const BackendOptions& backendOptions,
                                   std::function<void()> gRenderCallbackDuringResize) override {
            mWindowBounds.size = appWindowParams.windowGeometry.size;
            return nullptr;
        }

        std::vector<ScreenBounds> GetMonitorsWorkAreas() override { return NullConfig::GetMonitorsWorkAreas();}

        bool IsWindowIconified(WindowPointer window) override { return false; }
        void RaiseWindow(WindowPointer window) override {}

        ScreenBounds GetWindowBounds(WindowPointer window) override {
            return mWindowBounds;
        }
        void SetWindowBounds(WindowPointer window, ScreenBounds windowBounds) override {
            mWindowBounds = windowBounds;
        }

        void WaitForEventTimeout(double timeout_seconds) override {
            std::this_thread::sleep_for(std::chrono::milliseconds((int)(timeout_seconds * 1000)));
        }

        float GetWindowSizeDpiScaleFactor(WindowPointer window) override { return NullConfig::GetWindowSizeDpiScaleFactor(); }

        void HideWindow(WindowPointer window) override {}
        void ShowWindow(WindowPointer window) override {}
        bool IsWindowHidden(WindowPointer window) override { return false; }

    private:
        ScreenBounds mWindowBounds = {};

    };
}} // namespace HelloImGui { namespace BackendApi
