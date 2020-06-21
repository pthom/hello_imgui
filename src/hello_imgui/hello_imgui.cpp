#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/internal/backend_impls/runner_factory.h"

namespace HelloImGui
{
void Run(RunnerParams& runnerParams)
{
    auto runner = FactorRunner(runnerParams);
    runner->Run();
}

void Run(VoidFunction guiFonction, ImVec2 windowSize, std::string windowTitle)
{
    RunnerParams runnerParams;
    runnerParams.callbacks.ShowGui = guiFonction;
    runnerParams.appWindowParams.windowSize = windowSize;
    runnerParams.appWindowParams.windowTitle = windowTitle;

    auto runner = FactorRunner(runnerParams);
    runner->Run();
}
}  // namespace HelloImGui