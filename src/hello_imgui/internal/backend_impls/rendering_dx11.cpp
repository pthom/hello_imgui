#ifdef HELLOIMGUI_HAS_DIRECTX11
#include "rendering_dx11.h"
#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/hello_imgui_error.h"

#include <cstdio>
#include <d3d11.h>
#include <iostream>

namespace HelloImGui
{
    Dx11Globals& GetDx11Globals()
    {
        static Dx11Globals instance;
        return instance;
    }

    // Helper functions
    namespace Dx11Setup
    {
        // Helper functions to use DirectX11
        bool CreateDeviceD3D(HWND hWnd)
        {
            std::cout << "DirectX11 support for HelloImGui is experimental and may fail with HighDPI. See rendering_dx11.h for more info\n";
            auto& gDx11Globals = GetDx11Globals();

            // Setup swap chain
            DXGI_SWAP_CHAIN_DESC sd;
            ZeroMemory(&sd, sizeof(sd));
            sd.BufferCount = 2;
            sd.BufferDesc.Width = 0;
            sd.BufferDesc.Height = 0;
            sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.BufferDesc.RefreshRate.Numerator = 60;
            sd.BufferDesc.RefreshRate.Denominator = 1;
            sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.OutputWindow = hWnd;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.Windowed = TRUE;
            sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

            UINT createDeviceFlags = 0;
            //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
            D3D_FEATURE_LEVEL featureLevel;
            const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
            if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &gDx11Globals.pSwapChain, &gDx11Globals.pd3dDevice, &featureLevel, &gDx11Globals.pd3dDeviceContext) != S_OK)
                return false;

            CreateRenderTarget();
            return true;
        }

        void CleanupDeviceD3D()
        {
            auto& gDx11Globals = GetDx11Globals();

            CleanupRenderTarget();
            if (gDx11Globals.pSwapChain) { gDx11Globals.pSwapChain->Release(); gDx11Globals.pSwapChain = nullptr; }
            if (gDx11Globals.pd3dDeviceContext) { gDx11Globals.pd3dDeviceContext->Release(); gDx11Globals.pd3dDeviceContext = nullptr; }
            if (gDx11Globals.pd3dDevice) { gDx11Globals.pd3dDevice->Release(); gDx11Globals.pd3dDevice = nullptr; }
        }

        void CreateRenderTarget()
        {
            auto& gDx11Globals = GetDx11Globals();

            ID3D11Texture2D* pBackBuffer;
            gDx11Globals.pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
            gDx11Globals.pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &gDx11Globals.mainRenderTargetView);
            pBackBuffer->Release();
        }

        void CleanupRenderTarget()
        {
            auto& gDx11Globals = GetDx11Globals();

            if (gDx11Globals.mainRenderTargetView) { gDx11Globals.mainRenderTargetView->Release(); gDx11Globals.mainRenderTargetView = nullptr; }
        }

    } // namespace Dx11Setup


    //  Impl of RenderingCallbacks_Impl_SwapBuffers
    void SwapDx11Buffers()
    {
        static ImVec2 viewportInitialSize = ImGui::GetMainViewport()->Size;
        ImVec2 currentViewportSize = ImGui::GetMainViewport()->Size;
        if ((currentViewportSize.x != viewportInitialSize.x) || (currentViewportSize.y != viewportInitialSize.y))
        {
            // Help appreciated!
            IM_ASSERT(false && "rendering_dx11.cpp::SwapDx11Buffers() The current implementation of Dx11 backend does "
                    "not support changing the window size!");
            // Once solved, AbstractRunner::DpiWindowSizeFactor() can be corrected by removing the ifdef
        }

        auto& gDx11Globals = GetDx11Globals();
        gDx11Globals.pSwapChain->Present(1, 0); // Present with vsync
    }

    RenderingCallbacksPtr PrepareBackendCallbacksCommonDx11()
    {
        auto callbacks = std::make_shared<RenderingCallbacks>();

        callbacks->Impl_NewFrame_3D = [callbacks]
        {
            ImGui_ImplDX11_NewFrame();
        };

        callbacks->Impl_Frame_3D_ClearColor  = [] (ImVec4) {};

        callbacks->Impl_RenderDrawData_To_3D = []
        {
            auto& gDx11Globals = GetDx11Globals();

            ImVec4 clear_color = HelloImGui::GetRunnerParams()->imGuiWindowParams.backgroundColor;
            const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
            gDx11Globals.pd3dDeviceContext->OMSetRenderTargets(1, &gDx11Globals.mainRenderTargetView, nullptr);
            gDx11Globals.pd3dDeviceContext->ClearRenderTargetView(gDx11Globals.mainRenderTargetView, clear_color_with_alpha);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        };


        callbacks->Impl_Shutdown_3D = []
        {
            ImGui_ImplDX11_Shutdown();
            Dx11Setup::CleanupDeviceD3D();
        };

        callbacks->Impl_ScreenshotRgb_3D     = [] { return ImageBuffer{}; };

        //         callbacks->Impl_GetFrameBufferSize;   //= [] { return ScreenSize{0, 0}; };

        callbacks->Impl_CreateFontTexture = [] { HIMG_ERROR("DX11 does not support font texture creation/deletion at runtim"); };
        callbacks->Impl_DestroyFontTexture = [] { HIMG_ERROR("DX11 does not support font texture creation/deletion at runtim"); };

        return callbacks;
    }

} // namespace HelloImGui

#endif // HELLOIMGUI_HAS_DIRECTX11
