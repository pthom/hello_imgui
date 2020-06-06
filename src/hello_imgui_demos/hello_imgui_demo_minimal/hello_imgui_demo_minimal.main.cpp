#include "hello_imgui/hello_imgui.h"

int main()
{
    HelloImGui::RunnerParams params;

    params.backendWindowParams.windowTitle = "Hello, Dear ImGui!";
    params.backendWindowParams.windowSize = {300.f, 200.f};

    params.imGuiWindowParams.defaultImGuiWindowType = HelloImGui::DefaultImGuiWindowType::NoDefaultWindow;
    params.imGuiWindowParams.backgroundColor = ImVec4{ 0.5f, 0.5f, 0.3f, 1.f };

    params.callbacks.ShowGui = []() {
        ImGui::Text("Hello, ImGui !");
        if (ImGui::Button("Quit"))
            return true;
        return false;
    };
    HelloImGui::Run(params);
    return 0;
}