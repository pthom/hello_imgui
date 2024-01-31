#if defined(HELLOIMGUI_HAS_DIRECTX11) && defined(HELLOIMGUI_USE_GLFW3)
#include "rendering_dx11.h"

#include "imgui_impl_dx11.h"
#include "imgui_impl_glfw.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "hello_imgui/hello_imgui_logger.h"
#include "hello_imgui/hello_imgui.h"


namespace HelloImGui
{
    // Below is implementation of RenderingCallbacks_LinkWindowingToRenderingBackend
    void PrepareGlfwForDx11(GLFWwindow* window)
    {
        auto & gDx11Globals = GetDx11Globals();

        HWND hwnd;
        {
            hwnd = glfwGetWin32Window(window);
        }

        // Initialize Direct3D
        if (! Dx11Setup::CreateDeviceD3D(hwnd))
        {
            Dx11Setup::CleanupDeviceD3D();
            IM_ASSERT(false && "CreateDeviceD3D failed");
            throw std::runtime_error("CreateDeviceD3D failed");
        }

        ImGui_ImplGlfw_InitForOther(window, true);
        ImGui_ImplDX11_Init(gDx11Globals.pd3dDevice, gDx11Globals.pd3dDeviceContext);
    }


    RenderingCallbacksPtr CreateBackendCallbacks_GlfwDx11()
    {
        auto callbacks = PrepareBackendCallbacksCommonDx11();

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

#endif // #if defined(HELLOIMGUI_HAS_DIRECTX11) && defined(HELLOIMGUI_USE_GLFW3)
