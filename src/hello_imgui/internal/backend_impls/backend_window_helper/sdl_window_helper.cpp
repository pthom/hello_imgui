#ifdef HELLOIMGUI_USE_SDL

#include "sdl_window_helper.h"
#include "window_geometry_helper.h"
#include "SDL.h"

namespace HelloImGui { namespace BackendApi
{
    WindowPointer SdlWindowHelper::CreateWindow(AppWindowParams &info, const BackendOptions& backendOptions)
    {
        auto searchMonitorResult = SearchForMonitor(GetMonitorsWorkAreas(), info.windowGeometry);
        int realMonitorIdx = searchMonitorResult.monitorIdx;
        if (searchMonitorResult.newPosition.has_value())
            info.windowGeometry.position = searchMonitorResult.newPosition.value();

        int window_flags = 0;

        auto monitorsWorksAreas = GetMonitorsWorkAreas();
        assert((realMonitorIdx >= 0) && (realMonitorIdx < monitorsWorksAreas.size()));

        ScreenSize& windowSize = info.windowGeometry.size;
        ScreenPosition & windowPosition = info.windowGeometry.position;
        WindowPositionMode positionMode = info.windowGeometry.positionMode;

        // Reduce size if too big compared to the monitor
        if (! info.windowGeometry.sizeAuto)
        {
            auto workArea = monitorsWorksAreas[realMonitorIdx];
            ForDim2(dim)
                if (windowSize[dim] > workArea.size[dim])
                    windowSize[dim] = workArea.size[dim];
        }

        auto fullScreenMode = info.windowGeometry.fullScreenMode;

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
            auto monitorBounds = monitorsWorksAreas[info.windowGeometry.monitorIdx];
            windowSize = monitorBounds.size;
            info.windowGeometry.position = monitorBounds.position;
            window_pos_sdl[0] = info.windowGeometry.position[0];
            window_pos_sdl[1] = info.windowGeometry.position[1];
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

        if (backendOptions.allowHighDpi)
            window_flags |= SDL_WINDOW_ALLOW_HIGHDPI;

        if (info.borderless)
            window_flags |= SDL_WINDOW_BORDERLESS;

        if (info.resizable)
            window_flags |= SDL_WINDOW_RESIZABLE;

        if (info.windowGeometry.windowSizeState == WindowSizeState::Standard)
            {}
        else if (info.windowGeometry.windowSizeState == WindowSizeState::Minimized)
            window_flags |= SDL_WINDOW_MINIMIZED;
        else if (info.windowGeometry.windowSizeState == WindowSizeState::Maximized)
            window_flags |= SDL_WINDOW_MAXIMIZED;

        auto window = SDL_CreateWindow(info.windowTitle.c_str(),
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
            SDL_GetDisplayUsableBounds(i, &rect);
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

    float SdlWindowHelper::GetWindowDpiScaleFactor(WindowPointer window)
    {
        return 1.f;
    }

    }} // namespace HelloImGui { namespace BackendApi

#endif // #ifdef HELLOIMGUI_USE_SDL
