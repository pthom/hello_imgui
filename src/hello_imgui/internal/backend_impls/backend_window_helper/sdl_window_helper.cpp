#ifdef HELLOIMGUI_USE_SDL

#include "sdl_window_helper.h"
#include "window_geometry_helper.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/win32_dpi_awareness.h"
#include "SDL.h"

#ifdef _WIN32
#include "SDL_syswm.h"
#ifdef CreateWindow
#undef CreateWindow
#endif
#include "winuser.h"
#endif

#include <cassert>


namespace HelloImGui { namespace BackendApi
{
    WindowPointer SdlWindowHelper::CreateWindow(AppWindowParams &appWindowParams, const BackendOptions& backendOptions)
    {
#ifdef _WIN32
        if (backendOptions.dpiAwareSdl)
            Internal::ImGui_ImplWin32_EnableDpiAwareness();
#endif

        auto searchMonitorResult = SearchForMonitor(GetMonitorsWorkAreas(), appWindowParams);
        int realMonitorIdx = searchMonitorResult.monitorIdx;
        if (searchMonitorResult.newPosition.has_value())
            appWindowParams.windowGeometry.position = searchMonitorResult.newPosition.value();

        int window_flags = 0;

        auto monitorsWorksAreas = GetMonitorsWorkAreas();
        assert((realMonitorIdx >= 0) && (realMonitorIdx < monitorsWorksAreas.size()));

        ScreenSize &windowSize = appWindowParams.windowGeometry.size;
        ScreenPosition &windowPosition = appWindowParams.windowGeometry.position;
        WindowPositionMode positionMode = appWindowParams.windowGeometry.positionMode;

        // Reduce size if too big compared to the monitor
        if (! appWindowParams.windowGeometry.sizeAuto)
        {
            auto workArea = monitorsWorksAreas[realMonitorIdx];
            ForDim2(dim)
                if (windowSize[dim] > workArea.size[dim])
                    windowSize[dim] = workArea.size[dim];
        }

        auto fullScreenMode = appWindowParams.windowGeometry.fullScreenMode;

        int window_pos_sdl[2];
        ForDim2(dim)
            window_pos_sdl[dim] = SDL_WINDOWPOS_UNDEFINED;
        if (positionMode == WindowPositionMode::FromCoords)
        {
            ForDim2(dim)
                window_pos_sdl[dim] = windowPosition[dim];
        }
        else if ( (positionMode == WindowPositionMode::MonitorCenter) && (fullScreenMode==FullScreenMode::NoFullScreen))
        {
            auto workArea = monitorsWorksAreas[realMonitorIdx];
            ForDim2(dim)
                window_pos_sdl[dim] = workArea.Center()[dim] - windowSize[dim] / 2;
        }

        if (fullScreenMode == FullScreenMode::FullMonitorWorkArea)
        {
            auto monitorBounds = monitorsWorksAreas[appWindowParams.windowGeometry.monitorIdx];
            windowSize = monitorBounds.size;
            appWindowParams.windowGeometry.position = monitorBounds.position;
            window_pos_sdl[0] = appWindowParams.windowGeometry.position[0];
            window_pos_sdl[1] = appWindowParams.windowGeometry.position[1];
        }
        else if (fullScreenMode == FullScreenMode::FullScreen)
            window_flags |= SDL_WINDOW_FULLSCREEN;
        else if (fullScreenMode == FullScreenMode::FullScreenDesktopResolution)
        {
            window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }

        if ((fullScreenMode == FullScreenMode::FullScreenDesktopResolution) ||(fullScreenMode==FullScreenMode::FullScreen))
        {
            window_pos_sdl[0] = SDL_WINDOWPOS_CENTERED_DISPLAY(realMonitorIdx);
            window_pos_sdl[1] = SDL_WINDOWPOS_CENTERED_DISPLAY(realMonitorIdx);
        }

        auto backend3DMode = backendOptions.backend3DMode;
        if (backend3DMode == Backend3dMode::OpenGl)
            window_flags |= SDL_WINDOW_OPENGL;
        else if (backend3DMode == Backend3dMode::Metal)
            window_flags |= SDL_WINDOW_METAL;
        else if (backend3DMode == Backend3dMode::Vulkan)
            window_flags |= SDL_WINDOW_VULKAN;
        else if (backend3DMode == Backend3dMode::No3d)
            {}
        else
            BACKEND_THROW("Unsupported backend3DMode");

        // If the window is created with the SDL_WINDOW_ALLOW_HIGHDPI flag,
        // its size in pixels may differ from its size in screen coordinates on platforms with high-DPI support
        // (e.g. iOS and macOS).
        if (backendOptions.dpiAwareSdl)
            window_flags |= SDL_WINDOW_ALLOW_HIGHDPI;

        if (appWindowParams.borderless)
            window_flags |= SDL_WINDOW_BORDERLESS;

        if (appWindowParams.resizable)
            window_flags |= SDL_WINDOW_RESIZABLE;

        if (appWindowParams.windowGeometry.windowSizeState == WindowSizeState::Standard)
            {}
        else if (appWindowParams.windowGeometry.windowSizeState == WindowSizeState::Minimized)
            window_flags |= SDL_WINDOW_MINIMIZED;
        else if (appWindowParams.windowGeometry.windowSizeState == WindowSizeState::Maximized)
            window_flags |= SDL_WINDOW_MAXIMIZED;

        window_flags |= SDL_WINDOW_HIDDEN;

        auto window = SDL_CreateWindow(appWindowParams.windowTitle.c_str(),
                                       window_pos_sdl[0], window_pos_sdl[1],
                                       windowSize[0], windowSize[1],
                                       window_flags);
        if (!window)
            BACKEND_THROW("BackendSdl::CreateWindow : SDL_CreateWindow returned NULL");

        SDL_GetWindowPosition(window, &windowPosition[0], &windowPosition[1]);
        SDL_GetWindowSize(window, &windowSize[0], &windowSize[1]);

        // printf("Final window size: %ix%i\n", windowSize[0], windowSize[1]);
        // printf("Final window position: %ix%i\n", windowPosition[0], windowPosition[1]);

        return (void *)(window);
    }

    std::vector<ScreenBounds> SdlWindowHelper::GetMonitorsWorkAreas()
    {
        int nbMonitors = SDL_GetNumVideoDisplays();
        std::vector<ScreenBounds> r;
        for (size_t i = 0; i < nbMonitors; ++i)
        {
            SDL_Rect rect;
            SDL_GetDisplayUsableBounds((int)i, &rect);
            ScreenBounds b{{rect.x, rect.y},
                           {rect.w, rect.h}};
            r.push_back(b);
        }
        return r;
    }

    bool SdlWindowHelper::IsWindowIconified(WindowPointer window)
    {
        auto window_flags = SDL_GetWindowFlags((SDL_Window *)(window));
        bool window_is_hidden = (window_flags & (SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED)) > 0;
        return window_is_hidden;
    }

    void SdlWindowHelper::RaiseWindow(WindowPointer window)
    {
        // Despite those efforts, the app does not come to the front under MacOS
        auto sdlWindow = (SDL_Window *)(window);
        SDL_ShowWindow(sdlWindow);
        SDL_FlashWindow(sdlWindow, SDL_FLASH_UNTIL_FOCUSED);
        SDL_RaiseWindow(sdlWindow);
    }

    ScreenBounds SdlWindowHelper::GetWindowBounds(WindowPointer window)
    {
        auto sdlWindow = (SDL_Window *)(window);
        int x, y, w, h;
        SDL_GetWindowSize(sdlWindow, &w, &h);
        SDL_GetWindowPosition(sdlWindow, &x, &y);
        ScreenBounds r{{x, y}, {w, h}};
        return r;
    }

    void SdlWindowHelper::SetWindowBounds(WindowPointer window, ScreenBounds windowBounds)
    {
        auto sdlWindow = (SDL_Window *)(window);

        // Size
        SDL_SetWindowSize(sdlWindow, windowBounds.size[0], windowBounds.size[1]);

        // Position
        SDL_SetWindowPosition(sdlWindow, windowBounds.position[0], windowBounds.position[1]);
    }

    void SdlWindowHelper::WaitForEventTimeout(double timeout_seconds)
    {
        int timeout_ms = (int)(timeout_seconds * 1000.);
        SDL_WaitEventTimeout(NULL, timeout_ms);
    }

    #ifdef _WIN32
    HWND SdlWindowToHwnd(WindowPointer window) 
    {
        SDL_Window *sdlwindow = (SDL_Window *)window;
        SDL_SysWMinfo info;
        SDL_VERSION(&info.version);
        bool success = SDL_GetWindowWMInfo(sdlwindow, &info);
        return info.info.win.window;
    }
    #endif
    
    float SdlWindowHelper::GetWindowSizeDpiScaleFactor(WindowPointer window)
    {
        // SDL does not support HighDPI
        // See https://github.com/libsdl-org/SDL/issues/2119

        // We have to implement manual workarounds for windows
        #ifdef _WIN32
        int dpi = GetDpiForWindow(SdlWindowToHwnd(window));
        float dpiScale = dpi / 96.f;
        return dpiScale;
        #else
        return 1.f;
        #endif

    }

    void SdlWindowHelper::HideWindow(WindowPointer window)
    {
        SDL_HideWindow((SDL_Window *) window);
    }
    void SdlWindowHelper::ShowWindow(WindowPointer window)
    {
        SDL_ShowWindow((SDL_Window *) window);
    }
    bool SdlWindowHelper::IsWindowHidden(WindowPointer window)
    {
        Uint32 flags = SDL_GetWindowFlags((SDL_Window  *)window);
        bool hidden = (flags & SDL_WINDOW_HIDDEN) != 0;
        return hidden;
    }

    }} // namespace HelloImGui { namespace BackendApi

#endif // #ifdef HELLOIMGUI_USE_SDL
