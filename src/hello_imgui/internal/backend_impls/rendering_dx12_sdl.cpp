#if defined(HELLOIMGUI_HAS_DIRECTX12) && defined(HELLOIMGUI_USE_SDL2)
#include "rendering_dx12.h"

#include "imgui_impl_dx12.h"
#include "imgui_impl_sdl2.h"

#include <SDL.h>
#include <SDL_syswm.h>

#include "hello_imgui/hello_imgui_logger.h"
#include "hello_imgui/hello_imgui.h"


namespace HelloImGui
{
    // Below is implementation of RenderingCallbacks_LinkWindowingToRenderingBackend
    void PrepareSdlForDx12(SDL_Window* window)
    {
        auto & gDxGlobals = GetDx12Globals();


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
        if (! Dx12Setup::CreateDeviceD3D(hwnd))
        {
            Dx12Setup::CleanupDeviceD3D();
            //::UnregisterClassW(wc.lpszClassName, wc.hInstance);
            IM_ASSERT(false && "CreateDeviceD3D failed");
            throw std::runtime_error("CreateDeviceD3D failed");
        }

        ImGui_ImplSDL2_InitForD3D(window);
        ImGui_ImplDX12_Init(gDxGlobals.pd3dDevice, NUM_FRAMES_IN_FLIGHT,
                            DXGI_FORMAT_R8G8B8A8_UNORM, gDxGlobals.pd3dSrvDescHeap,
                            gDxGlobals.pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
                            gDxGlobals.pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

    }


    RenderingCallbacksPtr CreateBackendCallbacks_SdlDx12()
    {
        auto callbacks = PrepareBackendCallbacksCommonDx12();

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

#endif // #if defined(HELLOIMGUI_HAS_DIRECTX12) && defined(HELLOIMGUI_USE_GLFW3)
