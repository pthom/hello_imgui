#include "hello_imgui/hello_imgui.h"
int main() {
    HelloImGui::RunnerParams params;
    params.backendWindowParams = {"Hello, Dear ImGui!", {300.f, 120.f}};
    int nb_cpp = 15, nb_cmake = 2;
    params.callbacks.ShowGui = [&]() {
        ImGui::TextWrapped("How many lines for this app that works on desktop and mbile devices?");
        ImGui::SliderInt("C++ lines", &nb_cpp, 0, 100);
        ImGui::InputInt("Cmake lines", &nb_cmake);
        if (ImGui::Button("Quit"))
            return true;
        return false;
    };
    HelloImGui::Run(params);
}
