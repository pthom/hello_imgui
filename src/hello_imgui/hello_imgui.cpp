#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/internal/backend_impls/runner_factory.h"

namespace HelloImGui
{
RunnerParams* gLastRunnerParams = nullptr;


void Run(RunnerParams& runnerParams)
{
    auto runner = FactorRunner(runnerParams);
    gLastRunnerParams = &runnerParams;
    runner->Run();
}

void Run(VoidFunction guiFunction, ImVec2 windowSize, std::string windowTitle)
{
    RunnerParams runnerParams;
    runnerParams.callbacks.ShowGui = guiFunction;
    runnerParams.appWindowParams.windowSize = windowSize;
    runnerParams.appWindowParams.windowTitle = windowTitle;
    gLastRunnerParams = &runnerParams;
    auto runner = FactorRunner(runnerParams);
    runner->Run();
}

RunnerParams* GetRunnerParams()
{
    if (gLastRunnerParams == nullptr)
        throw std::runtime_error("HelloImGui::GetRunnerParams() would return null. Did you call HelloImGui::Run()?");
    return gLastRunnerParams;
}

}  // namespace HelloImGui