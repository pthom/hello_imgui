#pragma once
#ifdef HELLOIMGUI_HAS_DIRECTX11

///
// Warning, there is a known issue with DirectX11 and HelloImGui:
// The framebuffer size is badly adjusted upon resizing the window (issue close to https://github.com/ocornut/imgui/issues/4870)
//
// Also, since AbstractRunner::HandleDpiOnSecondFrame() will resize the window on the second frame,
// widgets may appear too big at startup if using HighDPI.
//


#include "imgui_impl_dx11.h"
#include "hello_imgui/internal/backend_impls/rendering_callbacks.h"


#include <d3d11.h>

// For more info, see "Anatomy of an ImGui app lifecycle (cf ImGui examples)", in rendering_callbacks.h
struct GLFWwindow;
struct SDL_Window;


namespace HelloImGui
{
    // Functions from imgui examples/example_win32_directx12/main.cpp
    namespace Dx11Setup
    {
        bool CreateDeviceD3D(HWND hWnd);
        void CleanupDeviceD3D();
        void CreateRenderTarget();
        void CleanupRenderTarget();
    }

    // Data
    struct Dx11Globals
    {
        ID3D11Device*            pd3dDevice = nullptr;
        ID3D11DeviceContext*     pd3dDeviceContext = nullptr;
        IDXGISwapChain*          pSwapChain = nullptr;
        ID3D11RenderTargetView*  mainRenderTargetView = nullptr;
    };
    Dx11Globals& GetDx11Globals();

    //  Impl of RenderingCallbacks_Impl_SwapBuffers
    void SwapDx11Buffers();

    RenderingCallbacksPtr PrepareBackendCallbacksCommonDx11();

#ifdef HELLOIMGUI_USE_GLFW3
    RenderingCallbacksPtr CreateBackendCallbacks_GlfwDx11();
    void PrepareGlfwForDx11(GLFWwindow* window);
#endif
#ifdef HELLOIMGUI_USE_SDL2
    RenderingCallbacksPtr CreateBackendCallbacks_SdlDx11();
    void PrepareSdlForDx11(SDL_Window* window);
#endif

}

#endif // HELLOIMGUI_HAS_DIRECTX11
