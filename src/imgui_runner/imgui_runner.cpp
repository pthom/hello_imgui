#include "imgui_runner.h"

namespace ImGuiRunner
{

void Run(GuiLoopFunction guiLoopFunction,
         const AppWindowParams &appWindowParams)
{
    auto guiLoopWithExit = [&guiLoopFunction]() {
        guiLoopFunction();
        return false;
    };
    RunWithExit(guiLoopWithExit, appWindowParams);
}

}  // namespace ImGuiRunner
