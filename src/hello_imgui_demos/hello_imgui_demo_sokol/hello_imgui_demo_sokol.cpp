#include "hello_imgui/sokol/hello_imgui_sokol.h"

sapp_desc sokol_main(int argc, char* argv[])
{
    HelloImGui::RunnerParams params;
    params.appWindowParams.windowSize = {320.f, 120.f};
    params.appWindowParams.windowTitle = "Hello, Dear ImGui!";

    static int nb_cpp = 18, nb_cmake = 4;
    auto showGui = [&]() {
        ImGui::TextWrapped("How many lines for this app that works with sokol app?");
        ImGui::SliderInt(ICON_FA_FILE_CODE " C++ lines", &nb_cpp, 0, 100);
        ImGui::InputInt( ICON_FA_FILE_CODE " Cmake lines", &nb_cmake);
    };

    params.callbacks.ShowGui = showGui;
    return HelloImGui::Sokol::make_sapp_desc(params);
}
