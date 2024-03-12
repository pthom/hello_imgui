#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/icons_font_awesome_4.h"

int main(int, char *[]) {
    int nb_cpp = 10, nb_cmake = 2;
    auto showGui = [&]() {
        ImGui::TextWrapped("How many lines for this app that works on computers and mobile devices?");
        ImGui::SliderInt(ICON_FA_FILE_CODE " C++ lines", &nb_cpp, 0, 100);
        ImGui::InputInt( ICON_FA_FILE_CODE " Cmake lines", &nb_cmake);
    };
    HelloImGui::Run(showGui, "Hello, Dear ImGui!", true);
    return 0;
}
