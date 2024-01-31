#if defined(HELLOIMGUI_HAS_DIRECTX11) && defined(HELLOIMGUI_USE_SDL2)
#include "rendering_dx11.h"

#include "imgui_impl_dx11.h"
#include "imgui_impl_sdl2.h"

#include <SDL.h>
#include <SDL_syswm.h>

#include "hello_imgui/hello_imgui_logger.h"
#include "hello_imgui/hello_imgui.h"


namespace HelloImGui
{
    // Below is implementation of RenderingCallbacks_LinkWindowingToRenderingBackend
    void PrepareSdlForDx11(SDL_Window* window)
    {
        auto & gDx11Globals = GetDx11Globals();

        HWND hwnd;
        {
            SDL_SysWMinfo wmInfo;
            SDL_VERSION(&wmInfo.version);  // Initialize wmInfo to the version SDL is currently using
            if (SDL_GetWindowWMInfo(window, &wmInfo))
                hwnd = wmInfo.info.win.window;  // Here's your HWND
            else
            {
                IM_ASSERT(false && "Can get HWND from SDL_Window");
                throw std::runtime_error("Can get HWND from SDL_Window");
            }
        }

        // Initialize Direct3D
        if (! Dx11Setup::CreateDeviceD3D(hwnd))
        {
            Dx11Setup::CleanupDeviceD3D();
            IM_ASSERT(false && "CreateDeviceD3D failed");
            throw std::runtime_error("CreateDeviceD3D failed");
        }

        ImGui_ImplSDL2_InitForD3D(window);
        ImGui_ImplDX11_Init(gDx11Globals.pd3dDevice, gDx11Globals.pd3dDeviceContext);
    }


    RenderingCallbacksPtr CreateBackendCallbacks_SdlDx11()
    {
        auto callbacks = PrepareBackendCallbacksCommonDx11();

        callbacks->Impl_GetFrameBufferSize = []
        {
            auto window = (SDL_Window *)HelloImGui::GetRunnerParams()->backendPointers.sdlWindow;
            int width, height;
            SDL_GetWindowSize(window, &width, &height);
            return ScreenSize{width, height};
        };

        return callbacks;
    }

}

#endif // #if defined(HELLOIMGUI_HAS_DIRECTX11) && defined(HELLOIMGUI_USE_GLFW3)
