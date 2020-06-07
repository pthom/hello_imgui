#include "hello_imgui/hello_imgui.h"
int main() {
    HelloImGui::RunnerParams params;
    params.appWindowParams = {"Hello, Dear ImGui!", {320.f, 120.f}};
    int nb_cpp = 15, nb_cmake = 2;
    params.callbacks.ShowGui = [&]() {
        ImGui::TextWrapped("How many lines for this app that works on computers and mobile devices?");
        ImGui::SliderInt(ICON_FA_FILE_CODE " C++ lines", &nb_cpp, 0, 100);
        ImGui::InputInt( ICON_FA_FILE_CODE " Cmake lines", &nb_cmake);
        if (ImGui::Button("Quit"))
            return true;
        return false;
    };
    HelloImGui::Run(params);
}
