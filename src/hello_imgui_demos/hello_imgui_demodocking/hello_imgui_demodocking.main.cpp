#include "hello_imgui/hello_imgui.h"
int main() {
    HelloImGui::RunnerParams runnerParams;

    runnerParams.imGuiWindowParams.defaultImGuiWindowType =
        HelloImGui::DefaultImGuiWindowType::ProvideFullScreenDockSpace;


    runnerParams.dockingParams.dockingSplits =
        {
            { "MainDockSpace", "BottomSpace", ImGuiDir_Down, 0.25 },
            { "MainDockSpace", "LeftSpace", ImGuiDir_Left, 0.25 }

        };
    runnerParams.dockingParams.dockableWindows =
        {
            {"Main", "MainDockSpace", []() {
                ImGui::Text("Hello Main");
            }, true, false  },
            {"Logs", "BottomSpace", []() {
                ImGui::Text("Hello Logs");
            }},
            {"Left", "LeftSpace", []() {
                ImGui::Text("Hello Left");
            }, false, true}
        };

    runnerParams.callbacks.ShowGui = []() {
    };

    runnerParams.imGuiWindowParams.showMenuBar = true;
    runnerParams.callbacks.ShowMenus = []() {
        if (ImGui::BeginMenu("My Menu"))
        {
            ImGui::MenuItem("First item");
            ImGui::MenuItem("Second item");
            ImGui::MenuItem("Third item");
            ImGui::EndMenu();
        }
    };

    runnerParams.imGuiWindowParams.showStatusBar = true;
    // runnerParams.imGuiWindowParams.showStatus_Fps = false;
    runnerParams.callbacks.ShowStatus = [] {
        ImGui::Text("This is my status");
    };

    HelloImGui::Run(runnerParams);
}
