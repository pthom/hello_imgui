#include "imgui_runner.h"

namespace ImGuiRunner {

namespace DetailsGlfw {

void BeginFullWindow()
{
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin(
        "##FullWindowHiddenTitle",
        nullptr,
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar);
}


} // namespace DetailsGlfw

void RunWithExit(GuiLoopWithExitFunction guiLoopFunction, const AppWindowParams &params)
{
    (void)guiLoopFunction;
    (void)params;
}

} // namespace ImGuiRunner
