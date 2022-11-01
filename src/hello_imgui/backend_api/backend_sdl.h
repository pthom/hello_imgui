#include "hello_imgui/backend_api/backend_api.h"

#include <cmath>
#include <string>
#include <memory>
#include <iostream>


#define BACKEND_API_USE_SDL
#define BACKEND_API_USE_GLFW


#ifdef BACKEND_API_USE_SDL

#include "SDL.h"
#include "SDL_main.h"


namespace BackendApi
{
    class BackendSdl : public IBackend
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
} // namespace BackendApi

#endif // #ifdef BACKEND_API_USE_SDL
