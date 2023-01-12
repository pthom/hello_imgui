#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/internal/backend_impls/runner_factory.h"
#include "imgui_internal.h"
#include <deque>
#include <chrono>


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


// Private API, used internally by AppWindowScreenshotRgbBuffer()
AbstractRunner *GetAbstractRunner()
{
    return gLastRunner.get();
}

// Private API, not mentioned in headers!
std::string GlslVersion()
{
    std::string r = GetAbstractRunner()->Impl_GlslVersion();
    return r;
}


namespace ChronoShenanigans
{
    class ClockSeconds_
    {
    private:
        using Clock = std::chrono::high_resolution_clock;
        using second = std::chrono::duration<float, std::ratio<1>>;
        std::chrono::time_point<Clock> mStart;

    public:
        ClockSeconds_() : mStart(Clock::now()) {}

        float elapsed() const
        {
            return std::chrono::duration_cast<second>
                (Clock::now() - mStart).count();
        }
    };

    float ClockSeconds()
    {
        static ClockSeconds_ watch;
        return watch.elapsed();
    }

}

float FrameRate(float durationForMean)
{
    static std::deque<float> times;
    float now = ChronoShenanigans::ClockSeconds();
    times.push_back(now);
    if (times.size() <= 1)
        return 0.f;

    while (true)
    {
        float firstTime = times.front();
        float age = now - firstTime;
        if ((age > durationForMean) && (times.size() >= 3))
            times.pop_front();
        else
            break;
    }

    float totalTime = times.back() - times.front();
    int nbFrames = times.size();
    float fps = 1. / (totalTime / (float) (nbFrames - 1));
    return fps;
}

}  // namespace HelloImGui