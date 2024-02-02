#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/internal/backend_impls/runner_factory.h"
#include "hello_imgui/internal/menu_statusbar.h"
#include "hello_imgui/internal/docking_details.h"
#include "imgui_internal.h"
#include <deque>
#include <set>
#include <chrono>


namespace HelloImGui
{
RunnerParams* gLastRunnerParams = nullptr;
std::unique_ptr<AbstractRunner> gLastRunner;


bool _CheckAdditionLayoutNamesUniqueness(RunnerParams &runnerParams)
{
    std::set<std::string> names_set;
    names_set.insert(runnerParams.dockingParams.layoutName);
    for (const auto& layout: runnerParams.alternativeDockingLayouts)
        names_set.insert(layout.layoutName);

    bool areNamesUnique = (names_set.size() == 1 + runnerParams.alternativeDockingLayouts.size());
    if (!areNamesUnique)
        fprintf(stderr, R"(
            Please give unique names to your layouts!
                -> Set layoutName inside runnerParams.dockingParams and runnerParams.alternativeDockingLayouts
        )");
    return areNamesUnique;
}

void Run(RunnerParams& runnerParams)
{
    IM_ASSERT(_CheckAdditionLayoutNamesUniqueness(runnerParams));
    gLastRunner = FactorRunner(runnerParams);
    IM_ASSERT(gLastRunner && "Could not factor runner!");
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

void SwitchLayout(const std::string& layoutName)
{
    gLastRunner->LayoutSettings_SwitchLayout(layoutName);
}

std::string CurrentLayoutName()
{
    return gLastRunnerParams->dockingParams.layoutName;
}


// Private API, used internally by AppWindowScreenshotRgbBuffer()
AbstractRunner *GetAbstractRunner()
{
    return gLastRunner.get();
}

// Private API, not mentioned in headers!
std::string GlslVersion()
{
#ifdef HELLOIMGUI_HAS_OPENGL
    std::string r = GetAbstractRunner()->Impl_GlslVersion();
    return r;
#else
    bool GlslVersionNotAvailable = false;
    IM_ASSERT(GlslVersionNotAvailable);
    return "";
#endif
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
    int nbFrames = (int)times.size();
    float fps = 1.f / (totalTime / (float) (nbFrames - 1));
    return fps;
}

std::string PlatformBackendTypeToString(PlatformBackendType platformBackendType)
{
    if (platformBackendType == PlatformBackendType::Glfw)
        return "Glfw";
    else if (platformBackendType == PlatformBackendType::Sdl)
        return "Sdl";
    else
        return "Unknown platform backend";
}

std::string RendererBackendTypeToString(RendererBackendType rendererBackendType)
{
    if (rendererBackendType == RendererBackendType::OpenGL3)
        return "OpenGL3";
    else if (rendererBackendType == RendererBackendType::Vulkan)
        return "Vulkan";
    else if (rendererBackendType == RendererBackendType::Metal)
        return "Metal";
    else if (rendererBackendType == RendererBackendType::DirectX11)
        return "DirectX11";
    else if (rendererBackendType == RendererBackendType::DirectX12)
        return "DirectX12";
    else
        return "Unknown renderer backend";
}

std::string GetBackendDescription()
{
    const auto& params = GetRunnerParams();
    std::string platformBackend = PlatformBackendTypeToString(params->platformBackendType);
    std::string rendererBackend = RendererBackendTypeToString(params->rendererBackendType);
    return platformBackend + " - " + rendererBackend;
}


#ifdef HELLOIMGUI_WITH_TEST_ENGINE
extern ImGuiTestEngine *GHImGuiTestEngine;
ImGuiTestEngine* GetImGuiTestEngine() { return GHImGuiTestEngine; }
#else
ImGuiTestEngine* GetImGuiTestEngine() { return nullptr; }
#endif


void SaveUserPref(const std::string& userPrefName, const std::string& userPrefContent)
{
    gLastRunner->SaveUserPref(userPrefName, userPrefContent);
}

std::string LoadUserPref(const std::string& userPrefName)
{
    return gLastRunner->LoadUserPref(userPrefName);
}


void ShowViewMenu(RunnerParams & runnerParams)
{
    DockingDetails::ShowViewMenu(runnerParams);
}
void ShowAppMenu(RunnerParams & runnerParams)
{
    Menu_StatusBar::ShowDefaultAppMenu_Quit(runnerParams);
}

}  // namespace HelloImGui