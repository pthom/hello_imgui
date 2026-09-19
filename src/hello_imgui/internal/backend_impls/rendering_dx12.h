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

    // Simple free list based allocator for SRV descriptors
    // (from imgui examples/example_win32_directx12/main.cpp)
    // imgui_impl_dx12 allocates one descriptor per texture (the font atlas may be recreated at any time)
    struct Dx12DescriptorHeapAllocator
    {
        ID3D12DescriptorHeap*       Heap = nullptr;
        D3D12_DESCRIPTOR_HEAP_TYPE  HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
        D3D12_CPU_DESCRIPTOR_HANDLE HeapStartCpu = {};
        D3D12_GPU_DESCRIPTOR_HANDLE HeapStartGpu = {};
        UINT                        HeapHandleIncrement = 0;
        ImVector<int>               FreeIndices;

        void Create(ID3D12Device* device, ID3D12DescriptorHeap* heap)
        {
            IM_ASSERT(Heap == nullptr && FreeIndices.empty());
            Heap = heap;
            D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
            HeapType = desc.Type;
            HeapStartCpu = Heap->GetCPUDescriptorHandleForHeapStart();
            HeapStartGpu = Heap->GetGPUDescriptorHandleForHeapStart();
            HeapHandleIncrement = device->GetDescriptorHandleIncrementSize(HeapType);
            FreeIndices.reserve((int)desc.NumDescriptors);
            for (int n = desc.NumDescriptors; n > 0; n--)
                FreeIndices.push_back(n - 1);
        }
        void Destroy()
        {
            Heap = nullptr;
            FreeIndices.clear();
        }
        void Alloc(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
        {
            IM_ASSERT(FreeIndices.Size > 0);
            int idx = FreeIndices.back();
            FreeIndices.pop_back();
            out_cpu_desc_handle->ptr = HeapStartCpu.ptr + (idx * HeapHandleIncrement);
            out_gpu_desc_handle->ptr = HeapStartGpu.ptr + (idx * HeapHandleIncrement);
        }
        void Free(D3D12_CPU_DESCRIPTOR_HANDLE cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_desc_handle)
        {
            int cpu_idx = (int)((cpu_desc_handle.ptr - HeapStartCpu.ptr) / HeapHandleIncrement);
            int gpu_idx = (int)((gpu_desc_handle.ptr - HeapStartGpu.ptr) / HeapHandleIncrement);
            IM_ASSERT(cpu_idx == gpu_idx);
            FreeIndices.push_back(cpu_idx);
        }
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
    constexpr int                    SRV_HEAP_SIZE = 64;

    // Data
    struct Dx12Globals
    {
        // Data
        FrameContext frameContext[NUM_FRAMES_IN_FLIGHT] = {};
        UINT frameIndex = 0;

        ID3D12Device*                pd3dDevice = nullptr;
        ID3D12DescriptorHeap*        pd3dRtvDescHeap = nullptr;
        ID3D12DescriptorHeap*        pd3dSrvDescHeap = nullptr;
        Dx12DescriptorHeapAllocator  pd3dSrvDescHeapAlloc;
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
