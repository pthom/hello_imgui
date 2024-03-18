#ifdef HELLOIMGUI_HAS_OPENGL
#include "rendering_opengl3.h"

#include "hello_imgui/hello_imgui_include_opengl.h"
#include "hello_imgui/internal/backend_impls/opengl_setup_helper/opengl_screenshot.h"
#include "imgui_impl_opengl3.h"

namespace HelloImGui
{
    RenderingCallbacksPtr CreateBackendCallbacks_OpenGl3()
    {
        auto callbacks = std::make_shared<RenderingCallbacks>();

        callbacks->Impl_NewFrame_3D = [] {
            ImGui_ImplOpenGL3_NewFrame();
        };

        callbacks->Impl_RenderDrawData_To_3D = [] {
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        };

        callbacks->Impl_ScreenshotRgb_3D = []() {
            return OpenglScreenshotRgb();
        };

        callbacks->Impl_Frame_3D_ClearColor = [](ImVec4 clear_color) {
            auto& io = ImGui::GetIO();
            glViewport(0, 0, static_cast<int>(io.DisplaySize.x), static_cast<int>(io.DisplaySize.y));
            glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
        };

        callbacks->Impl_Shutdown_3D = [] {
            ImGui_ImplOpenGL3_Shutdown();
        };

        callbacks->Impl_CreateFontTexture = ImGui_ImplOpenGL3_CreateFontsTexture;
        callbacks->Impl_DestroyFontTexture = ImGui_ImplOpenGL3_DestroyFontsTexture;

        return callbacks;
    }

}

#endif