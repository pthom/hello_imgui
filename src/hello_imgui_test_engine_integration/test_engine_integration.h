#include "hello_imgui/runner_params.h"

struct ImGuiTestEngine;

namespace HelloImGui
{
    void _AddTestEngineCallbacks(RunnerParams* runnerParams);
    ImGuiTestEngine* GetImGuiTestEngine();
}
