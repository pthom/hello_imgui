#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/internal/backend_impls/runner_factory.h"
#include "imgui_internal.h"

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

float EmSize()
{
    IM_ASSERT(GImGui != NULL); // EmSize can only be called after ImGui context was created!
    float r = ImGui::GetFontSize();
    return r;
}

float EmSize(float nbLines)
{
    return ImGui::GetFontSize() * nbLines;
}

ImVec2 EmVec2(float x, float y)
{
    IM_ASSERT(GImGui != NULL);
    float k = ImGui::GetFontSize();
    return ImVec2(k * x, k * y);
}

ImVec2 EmVec2(ImVec2 v)
{
    IM_ASSERT(GImGui != NULL);
    float k = ImGui::GetFontSize();
    return ImVec2(k * v.x, k * v.y);
}


// Private API, used internally by AppWindowScreenshotRgbBuffer()
AbstractRunner *GetRunner()
{
    return gLastRunner.get();
}

// Private API, not mentioned in headers!
std::string GlslVersion()
{
    std::string r = GetRunner()->Impl_GlslVersion();
    return r;
}

}  // namespace HelloImGui