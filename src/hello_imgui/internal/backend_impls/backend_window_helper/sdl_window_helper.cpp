#ifdef HELLOIMGUI_USE_SDL2

#include "sdl_window_helper.h"
#include "window_geometry_helper.h"
#include "hello_imgui/internal/backend_impls/backend_window_helper/win32_dpi_awareness.h"
#include "SDL.h"

#include <cassert>

#ifdef __APPLE__
#include <TargetConditionals.h>
#include "hello_imgui/internal/platform/getAppleBundleResourcePath.h"
#endif


namespace HelloImGui { namespace BackendApi
{
    std::function<void()> gRenderCallbackDuringResize_Sdl;

    static int resizingEventWatcher(void* data, SDL_Event* event)
    {
        // See https://github.com/pthom/hello_imgui/issues/112
        // This may trigger a reentrant call to
        //    AbstractRunner::CreateFramesAndRender()
        // By default, this is disabled.
        // See pref AppWindowParams.repaintDuringResize_GotchaReentrantRepaint
        if (event->type == SDL_WINDOWEVENT &&
            event->window.event == SDL_WINDOWEVENT_RESIZED) {
            SDL_Window* win = SDL_GetWindowFromID(event->window.windowID);
            if (win == (SDL_Window*)data)
            {
                if (gRenderCallbackDuringResize_Sdl)
                    gRenderCallbackDuringResize_Sdl();
            }
        }
        return 0;
    }


    WindowPointer SdlWindowHelper::CreateWindow(AppWindowParams &appWindowParams, const BackendOptions& backendOptions,
                                                std::function<void()> renderCallbackDuringResize)
    {
        gRenderCallbackDuringResize_Sdl = renderCallbackDuringResize;

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

        // Note: This is RenderingCallbacks_Impl_Hint_WindowingBackend
        auto rendererBackendType = backendOptions.rendererBackendType;
        if (rendererBackendType == RendererBackendType::OpenGL3)
            window_flags |= SDL_WINDOW_OPENGL;
        else if (rendererBackendType == RendererBackendType::Metal)
        {
            // Inform SDL that we will be using metal for rendering. Without this hint initialization of metal renderer may fail.
            SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
            window_flags |= SDL_WINDOW_METAL;
        }
        else if (rendererBackendType == RendererBackendType::Vulkan)
            window_flags |= SDL_WINDOW_VULKAN;
        else if (rendererBackendType == RendererBackendType::DirectX11) {}
        else if (rendererBackendType == RendererBackendType::DirectX12) {}
        else
            BACKEND_THROW("Unsupported backend3DMode");

        // From 2.0.18: Enable native IME.
        #ifdef SDL_HINT_IME_SHOW_UI
        SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
        #endif

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

        SDL_AddEventWatch(resizingEventWatcher, window);


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

    float SdlWindowHelper::GetWindowSizeDpiScaleFactor(WindowPointer window)
    {
        #if TARGET_OS_MAC // is true for any software platform that's derived from macOS, which includes iOS, watchOS, and tvOS
            // with apple, the OS handles the scaling, so we don't need to do anything
            return 1.f;
        #elif defined(_WIN32) || defined(__ANDROID__)
            // Shall linux use this branch? To be confirmed
            // under linux: SDL_GetDisplayDPI can fail (at least on ubuntu 22 / Parallels VM)
            float ddpi, hdpi, vdpi;
            if (SDL_GetDisplayDPI(0, &ddpi, &hdpi, &vdpi) != 0)
            {
                std::cerr << "GetWindowSizeDpiScaleFactor: Failed to get DPI: " << SDL_GetError() << std::endl;
                return 1.f;
            }
            #ifndef __ANDROID__
                // normally images are displayed at 96PPI on a desktop screen.
                // That means, that independently of the screen DPI, the image size in millimeters
                // corresponds to a hypothetical 9DPPI screen.
                float targetPpi = 96.f;
            #else
                // However on a mobile device, we choose to display them at 140PPI (so that widgets and images appear smaller)
                float targetPpi = 140.f;
            #endif
            return ddpi / targetPpi;
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

#endif // #ifdef HELLOIMGUI_USE_SDL2
