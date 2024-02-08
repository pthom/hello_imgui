#include "hello_imgui/hello_imgui.h"
#include "imgui.h"

int main()
{
    HelloImGui::RunnerParams runnerParams;
    runnerParams.platformBackendType = HelloImGui::PlatformBackendType::Glfw; // Sdl fails with Vulkan
    runnerParams.callbacks.ShowGui = []() {
        #ifdef HELLOIMGUI_HAS_OPENGL
        ImGui::Text("HELLOIMGUI_HAS_OPENGL");
        #endif
        #ifdef HELLOIMGUI_HAS_METAL
        ImGui::Text("HELLOIMGUI_HAS_METAL");
        #endif
        #ifdef HELLOIMGUI_HAS_VULKAN
        ImGui::Text("HELLOIMGUI_HAS_VULKAN");
        #endif
        #ifdef HELLOIMGUI_HAS_DIRECTX11
        ImGui::Text("HELLOIMGUI_HAS_DIRECTX11");
        #endif
        #ifdef HELLOIMGUI_HAS_DIRECTX12
        ImGui::Text("HELLOIMGUI_HAS_DIRECTX12");
        #endif

        ImGui::ShowDemoWindow();

    };
    HelloImGui::Run(runnerParams);
}