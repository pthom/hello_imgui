#include "hello_imgui/hello_imgui.h"
#include "imgui.h"

int main()
{
    HelloImGui::RunnerParams runnerParams;

    auto gui = []() {
        ImGui::Text("Hello, world!");
        ImGui::ShowDemoWindow();
        ImGui::Text("FrameRate (ImGui): %.3f", ImGui::GetIO().Framerate);
        ImGui::Text("FrameRate (HelloImGui): %.3f", HelloImGui::FrameRate());
    };
    runnerParams.callbacks.ShowGui = gui;
    runnerParams.remoteParams.enableRemoting = true;
//    runnerParams.fpsIdling.enableIdling = false;
    HelloImGui::Run(runnerParams);
}