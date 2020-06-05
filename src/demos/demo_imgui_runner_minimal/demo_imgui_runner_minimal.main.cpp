#include "imgui_runner.h"
int main() {
    int nb_cpp = 10, nb_cmake = 2;
    auto gui_loop = [&]() {
        ImGui::TextWrapped("How many lines for this app that works on desktop and mbile devices?");
        ImGui::SliderInt("C++ lines", &nb_cpp, 0, 100);
        ImGui::InputInt("Cmake lines", &nb_cmake);
    };
    ImGuiRunner::Run(gui_loop, {"qtimgui minimal demo", 300, 90});
}
