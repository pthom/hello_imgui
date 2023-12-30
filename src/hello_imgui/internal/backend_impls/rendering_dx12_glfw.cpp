#if defined(HELLOIMGUI_HAS_DIRECTX12) && defined(HELLOIMGUI_USE_GLFW3)
#include "rendering_dx12.h"

#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_glfw.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


#include "hello_imgui/hello_imgui_logger.h"
#include "hello_imgui/hello_imgui.h"


namespace HelloImGui
{
    // Below is implementation of RenderingCallbacks_LinkWindowingToRenderingBackend
    void PrepareGlfwForDx12(GLFWwindow* window)
    {
        auto & gDxGlobals = GetDx12Globals();

        HWND hwnd = glfwGetWin32Window(window);
        // Initialize Direct3D
        if (! Dx12Setup::CreateDeviceD3D(hwnd))
        {
            Dx12Setup::CleanupDeviceD3D();
            //::UnregisterClassW(wc.lpszClassName, wc.hInstance);
            IM_ASSERT(false && "CreateDeviceD3D failed");
            throw std::runtime_error("CreateDeviceD3D failed");
        }

        ImGui_ImplDX12_Init(gDxGlobals.pd3dDevice, NUM_FRAMES_IN_FLIGHT,
                            DXGI_FORMAT_R8G8B8A8_UNORM, gDxGlobals.pd3dSrvDescHeap,
                            gDxGlobals.pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
                            gDxGlobals.pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

    }


    RenderingCallbacksPtr CreateBackendCallbacks_GlfwDx12()
    {
        auto callbacks = PrepareBackendCallbacksCommonDx12();

        callbacks->Impl_GetFrameBufferSize = []
        {
            auto window = (GLFWwindow *) HelloImGui::GetRunnerParams()->backendPointers.glfwWindow;
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            return ScreenSize{width, height};
        };

        return callbacks;
    }

}

#endif // #if defined(HELLOIMGUI_HAS_DIRECTX12) && defined(HELLOIMGUI_USE_GLFW3)
