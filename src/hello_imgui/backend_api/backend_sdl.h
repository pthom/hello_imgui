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
    class BackendSdlWindow : public IBackendWindow
    {
    public:
        SDL_Window *mWindow;
        bool mShouldWindowClose = false;

        BackendSdlWindow(SDL_Window *window) : mWindow(window)
        {}

        ~BackendSdlWindow()
        {}
    };


    class BackendSdl : public IBackend
    {
    private:
        BackendSdlWindow *GetBackendSdlWindow(IBackendWindow *bw);
        SDL_Window *GetSdlWindow(IBackendWindow *bw);

    public:
        void Init() override;

        void DeInit() override;

        IBackendWindow *CreateWindow(WindowOptions &info) override;
        void DestroyWindow(IBackendWindow *window) override;

        size_t GetNbMonitors() override;
        ScreenBounds GetOneMonitorWorkArea(int monitorIndex) override;

        bool IsWindowIconified(IBackendWindow *window) override;
        bool ShouldWindowClose(IBackendWindow *window) override;

        void RaiseWindow(IBackendWindow *window) override;

        ScreenBounds GetWindowBounds(IBackendWindow *window) override;
        void SetWindowBounds(IBackendWindow *window, ScreenBounds windowBounds) override;

        void WaitForEvent(IBackendWindow *window, int timeOutMilliseconds) override;
        void PollEvents(IBackendWindow *bw, const AnyEventCallback &anyEventCallback = EventCallback_NoOp) override;
    };
} // namespace BackendApi

#endif // #ifdef BACKEND_API_USE_SDL
