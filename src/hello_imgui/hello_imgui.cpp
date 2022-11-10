#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/internal/backend_impls/runner_factory.h"

namespace HelloImGui
{
RunnerParams* gLastRunnerParams = nullptr;
std::unique_ptr<AbstractRunner> gLastRunner;

void Run(RunnerParams& runnerParams)
{
    gLastRunner = FactorRunner(runnerParams);
    gLastRunnerParams = &runnerParams;
    gLastRunner->Run();
}

void Run(const SimpleRunnerParams& simpleRunnerParams)
{
    RunnerParams fullParams = simpleRunnerParams.ToRunnerParams();
    Run(fullParams);
}

void Run(
    const VoidFunction& guiFunction,
    const std::string& windowTitle,
    bool windowSizeAuto,
    bool windowRestorePreviousGeometry,
    const ScreenSize& windowSize,
    float fpsIdle
)
{
    SimpleRunnerParams params;
    params.guiFunction = guiFunction;
    params.windowTitle = windowTitle;
    params.windowSizeAuto = windowSizeAuto;
    params.windowRestorePreviousGeometry = windowRestorePreviousGeometry;
    params.windowSize = windowSize;
    params.fpsIdle = fpsIdle;
    Run(params);
}

RunnerParams* GetRunnerParams()
{
    if (gLastRunnerParams == nullptr)
        throw std::runtime_error("HelloImGui::GetRunnerParams() would return null. Did you call HelloImGui::Run()?");
    return gLastRunnerParams;
}

AbstractRunner *GetRunner()
{
    return gLastRunner.get();
}

}  // namespace HelloImGui