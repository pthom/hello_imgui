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

void Run(VoidFunction guiFunction, ImVec2 windowSize, std::string windowTitle, float fpsIdle)
{
    RunnerParams runnerParams;
    runnerParams.callbacks.ShowGui = guiFunction;
    runnerParams.appWindowParams.windowGeometry.size = {(int)windowSize.x, (int)windowSize.y};
    runnerParams.appWindowParams.windowTitle = windowTitle;
    runnerParams.fpsIdle = fpsIdle;

    gLastRunnerParams = &runnerParams;
    auto runner = FactorRunner(runnerParams);
    runner->Run();
}


void Run_AutoSize(VoidFunction guiFunction,
         std::string windowTitle,
         bool restoreLastWindowGeometry,
         ImVec2 windowSize,
        float fpsIdle
)
{
    RunnerParams runnerParams;

    runnerParams.appWindowParams.windowTitle = windowTitle;
    runnerParams.callbacks.ShowGui = guiFunction;

    if ((windowSize.x > 0.f) || (windowSize.y > 0.f))
        runnerParams.appWindowParams.windowGeometry.size = {(int)windowSize.x, (int)windowSize.y};
    else
        runnerParams.appWindowParams.windowGeometry.sizeAuto = true;

    runnerParams.appWindowParams.restorePreviousGeometry = restoreLastWindowGeometry;
    runnerParams.fpsIdle = fpsIdle;

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