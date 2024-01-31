#pragma once
#ifdef HELLOIMGUI_HAS_DIRECTX12

// Important DirectX12 was not tested
// (developed on a MacBook M1 computer, which does not support DirectX12)
// Testers and feedback is welcome.

#include "imgui_impl_dx12.h"
#include "hello_imgui/internal/backend_impls/rendering_callbacks.h"

#include <d3d12.h>
#include <dxgi1_4.h>


// For more info, see "Anatomy of an ImGui app lifecycle (cf ImGui examples)", in rendering_callbacks.h

struct GLFWwindow;
struct SDL_Window;


namespace HelloImGui
{
    struct FrameContext
    {
        ID3D12CommandAllocator* CommandAllocator;
        UINT64                  FenceValue;
    };

    // Functions from imgui examples/example_win32_directx12/main.cpp
    namespace Dx12Setup
    {
        // Forward declarations of helper functions
        bool CreateDeviceD3D(HWND hWnd);
        void CleanupDeviceD3D();
        void CreateRenderTarget();
        void CleanupRenderTarget();
        void WaitForLastSubmittedFrame();
        FrameContext* WaitForNextFrameResources();
    }

    constexpr int                    NUM_FRAMES_IN_FLIGHT = 3;
    constexpr int                    NUM_BACK_BUFFERS = 3;

    // Data
    struct Dx12Globals
    {
        // Data
        FrameContext frameContext[NUM_FRAMES_IN_FLIGHT] = {};
        UINT frameIndex = 0;

        ID3D12Device*                pd3dDevice = nullptr;
        ID3D12DescriptorHeap*        pd3dRtvDescHeap = nullptr;
        ID3D12DescriptorHeap*        pd3dSrvDescHeap = nullptr;
        ID3D12CommandQueue*          pd3dCommandQueue = nullptr;
        ID3D12GraphicsCommandList*   pd3dCommandList = nullptr;
        ID3D12Fence*                 fence = nullptr;
        HANDLE                       fenceEvent = nullptr;
        UINT64                       fenceLastSignaledValue = 0;
        IDXGISwapChain3*             pSwapChain = nullptr;
        HANDLE                       hSwapChainWaitableObject = nullptr;
        ID3D12Resource*              mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
        D3D12_CPU_DESCRIPTOR_HANDLE  mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};

        FrameContext*                frameCtxCurrent = nullptr;
    };
    Dx12Globals& GetDx12Globals();

    //  Impl of RenderingCallbacks_Impl_SwapBuffers
    void SwapDx12Buffers();

    RenderingCallbacksPtr PrepareBackendCallbacksCommonDx12();

#ifdef HELLOIMGUI_USE_GLFW3
//    RenderingCallbacksPtr CreateBackendCallbacks_GlfwDx12();
//    void PrepareGlfwForDx12(GLFWwindow* window);
#endif
#ifdef HELLOIMGUI_USE_SDL2
    RenderingCallbacksPtr CreateBackendCallbacks_SdlDx12();
    void PrepareSdlForDx12(SDL_Window* window);
#endif

}

#endif // HELLOIMGUI_HAS_DIRECTX12
