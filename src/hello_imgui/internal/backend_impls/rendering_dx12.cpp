// Important DirectX12 was not tested
// (developed on a MacBook M1 computer, which does not support DirectX12)
// Testers and feedback is welcome.

#ifdef HELLOIMGUI_HAS_DIRECTX12
#include "rendering_dx12.h"
#include "hello_imgui/hello_imgui.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>

#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

namespace HelloImGui
{
    Dx12Globals& GetDx12Globals()
    {
        static Dx12Globals instance;
        return instance;
    }

    // Helper functions
    namespace Dx12Setup
    {
        bool CreateDeviceD3D(HWND hWnd)
        {
            auto & gDxGlobals = GetDx12Globals();

            // Setup swap chain
            DXGI_SWAP_CHAIN_DESC1 sd;
            {
                ZeroMemory(&sd, sizeof(sd));
                sd.BufferCount = NUM_BACK_BUFFERS;
                sd.Width = 0;
                sd.Height = 0;
                sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
                sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                sd.SampleDesc.Count = 1;
                sd.SampleDesc.Quality = 0;
                sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
                sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
                sd.Scaling = DXGI_SCALING_STRETCH;
                sd.Stereo = FALSE;
            }

            // [DEBUG] Enable debug interface
#ifdef DX12_ENABLE_DEBUG_LAYER
            ID3D12Debug* pdx12Debug = nullptr;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
                pdx12Debug->EnableDebugLayer();
#endif

            // Create device
            D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
            if (D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&gDxGlobals.pd3dDevice)) != S_OK)
                return false;

                // [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef DX12_ENABLE_DEBUG_LAYER
            if (pdx12Debug != nullptr)
            {
                ID3D12InfoQueue* pInfoQueue = nullptr;
                gDxGlobals.pd3dDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
                pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
                pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
                pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
                pInfoQueue->Release();
                pdx12Debug->Release();
            }
#endif

            {
                D3D12_DESCRIPTOR_HEAP_DESC desc = {};
                desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                desc.NumDescriptors = NUM_BACK_BUFFERS;
                desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
                desc.NodeMask = 1;
                if (gDxGlobals.pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&gDxGlobals.pd3dRtvDescHeap)) != S_OK)
                    return false;

                SIZE_T rtvDescriptorSize = gDxGlobals.pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = gDxGlobals.pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
                for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
                {
                    gDxGlobals.mainRenderTargetDescriptor[i] = rtvHandle;
                    rtvHandle.ptr += rtvDescriptorSize;
                }
            }

            {
                D3D12_DESCRIPTOR_HEAP_DESC desc = {};
                desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                desc.NumDescriptors = 1;
                desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
                if (gDxGlobals.pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&gDxGlobals.pd3dSrvDescHeap)) != S_OK)
                    return false;
            }

            {
                D3D12_COMMAND_QUEUE_DESC desc = {};
                desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
                desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
                desc.NodeMask = 1;
                if (gDxGlobals.pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&gDxGlobals.pd3dCommandQueue)) != S_OK)
                    return false;
            }

            for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
                if (gDxGlobals.pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&gDxGlobals.frameContext[i].CommandAllocator)) != S_OK)
                    return false;

            if (gDxGlobals.pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, gDxGlobals.frameContext[0].CommandAllocator, nullptr, IID_PPV_ARGS(&gDxGlobals.pd3dCommandList)) != S_OK ||
                gDxGlobals.pd3dCommandList->Close() != S_OK)
                return false;

            if (gDxGlobals.pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&gDxGlobals.fence)) != S_OK)
                return false;

            gDxGlobals.fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (gDxGlobals.fenceEvent == nullptr)
                return false;

            {
                IDXGIFactory4* dxgiFactory = nullptr;
                IDXGISwapChain1* swapChain1 = nullptr;
                if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK)
                    return false;
                if (dxgiFactory->CreateSwapChainForHwnd(gDxGlobals.pd3dCommandQueue, hWnd, &sd, nullptr, nullptr, &swapChain1) != S_OK)
                    return false;
                if (swapChain1->QueryInterface(IID_PPV_ARGS(&gDxGlobals.pSwapChain)) != S_OK)
                    return false;
                swapChain1->Release();
                dxgiFactory->Release();
                gDxGlobals.pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);
                gDxGlobals.hSwapChainWaitableObject = gDxGlobals.pSwapChain->GetFrameLatencyWaitableObject();
            }

            CreateRenderTarget();
            return true;
        }

        void CleanupDeviceD3D()
        {
            auto & gDxGlobals = GetDx12Globals();

            CleanupRenderTarget();
            if (gDxGlobals.pSwapChain) { gDxGlobals.pSwapChain->SetFullscreenState(false, nullptr); gDxGlobals.pSwapChain->Release(); gDxGlobals.pSwapChain = nullptr; }
            if (gDxGlobals.hSwapChainWaitableObject != nullptr) { CloseHandle(gDxGlobals.hSwapChainWaitableObject); }
            for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
                if (gDxGlobals.frameContext[i].CommandAllocator) { gDxGlobals.frameContext[i].CommandAllocator->Release(); gDxGlobals.frameContext[i].CommandAllocator = nullptr; }
            if (gDxGlobals.pd3dCommandQueue) { gDxGlobals.pd3dCommandQueue->Release(); gDxGlobals.pd3dCommandQueue = nullptr; }
            if (gDxGlobals.pd3dCommandList) { gDxGlobals.pd3dCommandList->Release(); gDxGlobals.pd3dCommandList = nullptr; }
            if (gDxGlobals.pd3dRtvDescHeap) { gDxGlobals.pd3dRtvDescHeap->Release(); gDxGlobals.pd3dRtvDescHeap = nullptr; }
            if (gDxGlobals.pd3dSrvDescHeap) { gDxGlobals.pd3dSrvDescHeap->Release(); gDxGlobals.pd3dSrvDescHeap = nullptr; }
            if (gDxGlobals.fence) { gDxGlobals.fence->Release(); gDxGlobals.fence = nullptr; }
            if (gDxGlobals.fenceEvent) { CloseHandle(gDxGlobals.fenceEvent); gDxGlobals.fenceEvent = nullptr; }
            if (gDxGlobals.pd3dDevice) { gDxGlobals.pd3dDevice->Release(); gDxGlobals.pd3dDevice = nullptr; }

#ifdef DX12_ENABLE_DEBUG_LAYER
            IDXGIDebug1* pDebug = nullptr;
            if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
            {
                pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
                pDebug->Release();
            }
#endif
        }

        void CreateRenderTarget()
        {
            auto & gDxGlobals = GetDx12Globals();

            for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
            {
                ID3D12Resource* pBackBuffer = nullptr;
                gDxGlobals.pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
                gDxGlobals.pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, gDxGlobals.mainRenderTargetDescriptor[i]);
                gDxGlobals.mainRenderTargetResource[i] = pBackBuffer;
            }
        }

        void CleanupRenderTarget()
        {
            auto & gDxGlobals = GetDx12Globals();

            WaitForLastSubmittedFrame();

            for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
                if (gDxGlobals.mainRenderTargetResource[i]) { gDxGlobals.mainRenderTargetResource[i]->Release(); gDxGlobals.mainRenderTargetResource[i] = nullptr; }
        }

        void WaitForLastSubmittedFrame()
        {
            auto & gDxGlobals = GetDx12Globals();

            FrameContext* frameCtx = &gDxGlobals.frameContext[gDxGlobals.frameIndex % NUM_FRAMES_IN_FLIGHT];

            UINT64 fenceValue = frameCtx->FenceValue;
            if (fenceValue == 0)
                return; // No fence was signaled

            frameCtx->FenceValue = 0;
            if (gDxGlobals.fence->GetCompletedValue() >= fenceValue)
                return;

            gDxGlobals.fence->SetEventOnCompletion(fenceValue, gDxGlobals.fenceEvent);
            WaitForSingleObject(gDxGlobals.fenceEvent, INFINITE);
        }

        FrameContext* WaitForNextFrameResources()
        {
            auto & gDxGlobals = GetDx12Globals();

            UINT nextFrameIndex = gDxGlobals.frameIndex + 1;
            gDxGlobals.frameIndex = nextFrameIndex;

            HANDLE waitableObjects[] = { gDxGlobals.hSwapChainWaitableObject, nullptr };
            DWORD numWaitableObjects = 1;

            FrameContext* frameCtx = &gDxGlobals.frameContext[nextFrameIndex % NUM_FRAMES_IN_FLIGHT];
            UINT64 fenceValue = frameCtx->FenceValue;
            if (fenceValue != 0) // means no fence was signaled
            {
                frameCtx->FenceValue = 0;
                gDxGlobals.fence->SetEventOnCompletion(fenceValue, gDxGlobals.fenceEvent);
                waitableObjects[1] = gDxGlobals.fenceEvent;
                numWaitableObjects = 2;
            }

            WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

            return frameCtx;
        }

    } // namespace Dx12Setup


    //  Impl of RenderingCallbacks_Impl_SwapBuffers
    void SwapDx12Buffers()
    {
        auto & gDxGlobals = GetDx12Globals();

        gDxGlobals.pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync

        UINT64 fenceValue = gDxGlobals.fenceLastSignaledValue + 1;
        gDxGlobals.pd3dCommandQueue->Signal(gDxGlobals.fence, fenceValue);
        gDxGlobals.fenceLastSignaledValue = fenceValue;
        gDxGlobals.frameCtxCurrent->FenceValue = fenceValue;
    }

    RenderingCallbacksPtr PrepareBackendCallbacksCommonDx12()
    {
        auto callbacks = std::make_shared<RenderingCallbacks>();

        callbacks->Impl_NewFrame_3D = [callbacks]
        {
            ImGui_ImplDX12_NewFrame();
        };

        callbacks->Impl_Frame_3D_ClearColor  = [] (ImVec4) {};

        callbacks->Impl_RenderDrawData_To_3D = []
        {
            auto & gDxGlobals = GetDx12Globals();

            gDxGlobals.frameCtxCurrent = Dx12Setup::WaitForNextFrameResources();
            UINT backBufferIdx = gDxGlobals.pSwapChain->GetCurrentBackBufferIndex();
            gDxGlobals.frameCtxCurrent->CommandAllocator->Reset();

            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource   = gDxGlobals.mainRenderTargetResource[backBufferIdx];
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
            gDxGlobals.pd3dCommandList->Reset(gDxGlobals.frameCtxCurrent->CommandAllocator, nullptr);
            gDxGlobals.pd3dCommandList->ResourceBarrier(1, &barrier);

            // Render Dear ImGui graphics
            ImVec4 clear_color = HelloImGui::GetRunnerParams()->imGuiWindowParams.backgroundColor;
            const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
            gDxGlobals.pd3dCommandList->ClearRenderTargetView(gDxGlobals.mainRenderTargetDescriptor[backBufferIdx], clear_color_with_alpha, 0, nullptr);
            gDxGlobals.pd3dCommandList->OMSetRenderTargets(1, &gDxGlobals.mainRenderTargetDescriptor[backBufferIdx], FALSE, nullptr);
            gDxGlobals.pd3dCommandList->SetDescriptorHeaps(1, &gDxGlobals.pd3dSrvDescHeap);
            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), gDxGlobals.pd3dCommandList);
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
            gDxGlobals.pd3dCommandList->ResourceBarrier(1, &barrier);
            gDxGlobals.pd3dCommandList->Close();

            gDxGlobals.pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&gDxGlobals.pd3dCommandList);
        };


        callbacks->Impl_Shutdown_3D = []
        {
            ImGui_ImplDX12_Shutdown();
            Dx12Setup::CleanupDeviceD3D();
        };

        // callbacks->Impl_ScreenshotRgb_3D     = [] { return ImageBuffer{}; };
        // callbacks->Impl_GetFrameBufferSize;   //= [] { return ScreenSize{0, 0}; };

        callbacks->Impl_CreateFontTexture = [] { HIMG_ERROR("DX12 does not support font texture creation/deletion at runtim"); };
        callbacks->Impl_DestroyFontTexture = [] { HIMG_ERROR("DX12 does not support font texture creation/deletion at runtim"); };

        return callbacks;
    }

} // namespace HelloImGui

#endif // HELLOIMGUI_HAS_DIRECTX12
