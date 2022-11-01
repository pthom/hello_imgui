#include "hello_imgui/backend_api/backend_api.h"
#include <memory>
#include <iostream>

#define BACKEND_API_USE_GLFW


#ifdef BACKEND_API_USE_GLFW

#include <GLFW/glfw3.h>


namespace BackendApi
{
    class BackendGlfw: public IBackend
    {
    public:
        void Init() override;
        void DeInit() override;

        WindowPointer CreateWindow(WindowOptions &info) override;

        size_t GetNbMonitors() override;
        ScreenBounds GetOneMonitorWorkArea(int monitorIndex) override;

        bool IsWindowIconified(WindowPointer window) override;
        void RaiseWindow(WindowPointer window) override;

        ScreenBounds GetWindowBounds(WindowPointer window) override;
        void SetWindowBounds(WindowPointer window, ScreenBounds windowBounds) override;
    };

#endif // #ifdef BACKEND_API_USE_GLFW
}

