#include "hello_imgui/hello_imgui.h"
int main() {
    HelloImGui::RunnerParams runnerParams;

    runnerParams.imGuiWindowParams.defaultImGuiWindowType =
        HelloImGui::DefaultImGuiWindowType::ProvideFullScreenDockSpace;
    runnerParams.imGuiWindowParams.showMenuBar = true;

    runnerParams.dockingParams.dockingSplits =
        {
            { "MainDockSpace", "BottomSpace", ImGuiDir_Down, 0.25 },
            { "MainDockSpace", "LeftSpace", ImGuiDir_Left, 0.25 }

        };
    runnerParams.dockingParams.windowDockingLocations =
        {
            {"Main", "MainDockSpace"},
            {"Logs", "BottomSpace"},
            {"Left", "LeftSpace"}
        };

    runnerParams.callbacks.ShowGui = []() {
        ImGui::Begin("Main");
        ImGui::Text("Hello Main");
        ImGui::End();

        ImGui::Begin("Logs");
        ImGui::Text("Hello Logs");
        ImGui::End();

        ImGui::Begin("Left");
        ImGui::Text("Hello Left");
        ImGui::End();
    };

    runnerParams.callbacks.ShowMenus = []() {
        if (ImGui::BeginMenu("My Menu"))
        {
            ImGui::MenuItem("First item");
            ImGui::MenuItem("Second item");
            ImGui::MenuItem("Third item");
            ImGui::EndMenu();
        }

    };

    HelloImGui::Run(runnerParams);
}
