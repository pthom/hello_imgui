#include "hello_imgui.h"
#include "runner_factory.h"

namespace HelloImGui
{
    void Run(RunnerParams & runnerParams)
    {
        auto runner = FactorRunner();
        runner->params = runnerParams;
        runner->Run();
    }
}