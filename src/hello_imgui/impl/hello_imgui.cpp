#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/internal/backend_impls/runner_factory.h"
#include "hello_imgui/internal/menu_statusbar.h"
#include "hello_imgui/internal/docking_details.h"
#include "imgui_internal.h"
#include <deque>
#include <set>
#include <chrono>
#include <cstdio>
#include <optional>


namespace HelloImGui
{
std::string gMissingBackendErrorMessage = R"(

When running CMake, you should specify

 - At least one (or more) rendering backend (OpenGL3, Metal, Vulkan, DirectX11, DirectX12)
   Make your choice according to your needs and your target platforms, between:
      -DHELLOIMGUI_HAS_OPENGL3=ON    # This is the recommended choice, especially for beginners
      -DHELLOIMGUI_HAS_METAL=ON      # Apple only, advanced users only
      -DHELLOIMGUI_HAS_VULKAN=ON     # Advanced users only
      -DHELLOIMGUI_HAS_DIRECTX11=ON  # Windows only, still experimental
      -DHELLOIMGUI_HAS_DIRECTX12=ON  # Windows only, advanced users only, still experimental

 - At least one (or more) platform backend (SDL2, Glfw3):
  Make your choice according to your needs and your target platforms, between:
      -DHELLOIMGUI_USE_SDL2=ON
      -DHELLOIMGUI_USE_GLFW3=ON
)";


bool _CheckAdditionLayoutNamesUniqueness(const RunnerParams &runnerParams)
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


// =========================== Priv_SetupRunner ==================================
//
// Static instances to store the last runner and its parameters
// -------------------------------------------------------------
// gLastRunnerParamsOpt may contain a valid value if Renderer(const RunnerParams& ) was used
std::optional<RunnerParams> gLastRunnerParamsOpt;
// gLastRunnerParamsUserPointer may contain a valid value if Run(RunnerParams& ) was used
// (we may modify the user's runnerParams in this case)
static RunnerParams* gLastRunnerParamsUserPointer = nullptr;
// a pointer to the current AbstractRunner
static std::unique_ptr<AbstractRunner> gLastRunner;

static RunnerParams* Priv_CurrentRunnerParamsPtr()
{
    if (gLastRunnerParamsOpt.has_value())
        return &gLastRunnerParamsOpt.value();
    if (gLastRunnerParamsUserPointer != nullptr)
        return gLastRunnerParamsUserPointer;
    return nullptr;
}


// Only one instance of `Renderer` can exist at a time
// ---------------------------------------------------
static int gRendererInstanceCount = 0;

enum class SetupMode
{
    Renderer,
    Run
};

static SetupMode gSetupMode = SetupMode::Renderer;

static void Priv_TearDown();

static void Priv_SetupRunner(RunnerParams &passedUserParams, SetupMode setupMode)
{
#ifdef IMGUI_BUNDLE_BUILD_PYTHON
    // For python, we forgive the user for not calling TearDown() before calling Renderer()
    // since it might be due to a Python exception, which might be recoverable, if running
    // in a Python REPL, a notebook, or Pyodide.
    if (gRendererInstanceCount > 0)
    {
        printf("HelloImGui: calling TearDown() prior to Setup (probable prior exception in Python).\n");
        Priv_TearDown();
    }
#else
    if (gRendererInstanceCount > 0)
        throw std::runtime_error("Only one instance of `HelloImGui::Renderer` can exist at a time.");
#endif
    gSetupMode = setupMode;
    bool isUserPointer = (setupMode == SetupMode::Run);  // When using HelloImGui::Run, we may modify the user's runnerParams
    bool shallSetupTearDown = (setupMode == SetupMode::Renderer);  // When using HelloImGui::Renderer, we shall call Setup/TearDown()

    if (!isUserPointer)
        gLastRunnerParamsOpt = passedUserParams; // Store a copy of the user's runnerParams
    else
        gLastRunnerParamsUserPointer = &passedUserParams; // Store a pointer to the user's runnerParams

    IM_ASSERT(Priv_CurrentRunnerParamsPtr() != nullptr);
    RunnerParams &runnerParams = *Priv_CurrentRunnerParamsPtr();
    IM_ASSERT(_CheckAdditionLayoutNamesUniqueness(runnerParams));

    gLastRunner = FactorRunner(runnerParams);
    if (gLastRunner == nullptr)
    {
        fprintf(stderr, "HelloImGui::Renderer() failed to factor a runner!\n %s", gMissingBackendErrorMessage.c_str());
        IM_ASSERT(false && "HelloImGui::Renderer() failed to factor a runner!");
    }
    if (shallSetupTearDown)
        gLastRunner->Setup();
    gRendererInstanceCount++;
}

static void Priv_TearDown()
{
    IM_ASSERT(gLastRunner != nullptr && "HelloImGui::Renderer::~Renderer() called without a valid runner");
    bool shallSetupTearDown = (gSetupMode == SetupMode::Renderer);  // When using HelloImGui::Renderer, we shall call Setup/TearDown()
    if (shallSetupTearDown)
        gLastRunner->TearDown(false);
    gLastRunner = nullptr;
    gRendererInstanceCount = 0;
    gLastRunnerParamsOpt.reset();
    gLastRunnerParamsUserPointer = nullptr;
}


// =========================== ManualRender ==================================
namespace ManualRender
{
    // Enumeration to track the current state of the ManualRenderer
    enum class RendererStatus
    {
        NotInitialized,
        Initialized,
    };
    RendererStatus sCurrentStatus = RendererStatus::NotInitialized;

    // Changes the current status to Initialized if it was NotInitialized,
    // otherwise raises an error (assert or exception)
    void TrySwitchToInitialized()
    {
        if (sCurrentStatus == RendererStatus::Initialized)
            IM_ASSERT(false && "HelloImGui::ManualRender::SetupFromXXX() cannot be called while already initialized. Call TearDown() first.");
        sCurrentStatus = RendererStatus::Initialized;
    }

    // Changes the current status to NotInitialized if it was Initialized,
    // otherwise raises an error (assert or exception)
    void TrySwitchToNotInitialized()
    {
        if (sCurrentStatus == RendererStatus::NotInitialized)
            IM_ASSERT(false && "HelloImGui::ManualRender::TearDown() cannot be called while not initialized.");
        sCurrentStatus = RendererStatus::NotInitialized;
    }

    // Initializes the renderer with the full customizable `RunnerParams`.
    // A distinct copy of `RunnerParams` is stored internally.
    void SetupFromRunnerParams(const RunnerParams& runnerParams)
    {
        TrySwitchToInitialized();
        RunnerParams runnerParamsCopy = runnerParams;
        Priv_SetupRunner(runnerParamsCopy, SetupMode::Renderer);
    }

    // Initializes the renderer with `SimpleRunnerParams`.
    void SetupFromSimpleRunnerParams(const SimpleRunnerParams& simpleParams)
    {
        TrySwitchToInitialized();
        RunnerParams fullParams = simpleParams.ToRunnerParams();
        Priv_SetupRunner(fullParams, SetupMode::Renderer);
    }

    // Initializes the renderer with a simple GUI function and additional parameters.
    void SetupFromGuiFunction(
        const VoidFunction& guiFunction,
        const std::string& windowTitle,
        bool windowSizeAuto,
        bool windowRestorePreviousGeometry,
        const ScreenSize& windowSize,
        float fpsIdle
    )
    {
        TrySwitchToInitialized();
        SimpleRunnerParams params;
        params.guiFunction = guiFunction;
        params.windowTitle = windowTitle;
        params.windowSizeAuto = windowSizeAuto;
        params.windowRestorePreviousGeometry = windowRestorePreviousGeometry;
        params.windowSize = windowSize;
        params.fpsIdle = fpsIdle;
        RunnerParams fullParams = params.ToRunnerParams();
        Priv_SetupRunner(fullParams, SetupMode::Renderer);
    }

    // Renders the current frame. Should be called regularly to maintain the application's responsiveness.
    void Render()
    {
        IM_ASSERT(gLastRunner != nullptr && "HelloImGui::Renderer::Render() called without a valid runner");
        gLastRunner->CreateFramesAndRender();
    }

    // Tears down the renderer and releases all associated resources.
    // After calling `TearDown()`, the InitFromXXX can be called with new parameters.
    void TearDown()
    {
        TrySwitchToNotInitialized();
        Priv_TearDown();
    }

} // namespace ManualRender


// =========================== HelloImGui::Run ==================================

void Run(RunnerParams& runnerParams)
{
    Priv_SetupRunner(runnerParams, SetupMode::Run);
    gLastRunner->Run();
    Priv_TearDown();
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


// ============================== Utility functions ===============================

RunnerParams* GetRunnerParams()
{
    auto ptr = Priv_CurrentRunnerParamsPtr();
    if (ptr == nullptr)
        throw std::runtime_error("HelloImGui::GetRunnerParams() would return null. Did you call HelloImGui::Run()?");
    return ptr;
}

bool IsUsingHelloImGui()
{
    return Priv_CurrentRunnerParamsPtr() != nullptr;
}

void SwitchLayout(const std::string& layoutName)
{
    gLastRunner->LayoutSettings_SwitchLayout(layoutName);
}

std::string CurrentLayoutName()
{
    return GetRunnerParams()->dockingParams.layoutName;
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

static std::deque<float> gFrameTimes;

void _UpdateFrameRateStats()
{
    float now = ChronoShenanigans::ClockSeconds();
    gFrameTimes.push_back(now);

    size_t maxFrameCount = 300;
    while (gFrameTimes.size() > maxFrameCount)
        gFrameTimes.pop_front();
};

float FrameRate(float durationForMean)
{
    if (gFrameTimes.size() <= 1)
        return 0.f;

    float lastFrameTime = gFrameTimes.back();
    int lastFrameIdx = (int)gFrameTimes.size() - 1;

    // Go back in frame times to find the first frame that is not too old
    int i = (int)gFrameTimes.size() - 1;
    while (i > 0)
    {
        if (lastFrameTime - gFrameTimes[i] > durationForMean)
            break;
        --i;
    }
    if (i == lastFrameIdx)
        return 0.f;
    // printf("i=%d, lastFrameIdx=%d\n", i, lastFrameIdx);

    // Compute the mean frame rate
    float totalTime = lastFrameTime - gFrameTimes[i];
    int nbFrames = lastFrameIdx - i;
    float fps =  (float)nbFrames / totalTime;
    return fps;
}

std::string PlatformBackendTypeToString(PlatformBackendType platformBackendType)
{
    if (platformBackendType == PlatformBackendType::Glfw)
        return "Glfw";
    else if (platformBackendType == PlatformBackendType::Sdl)
        return "Sdl";
    else if (platformBackendType == PlatformBackendType::Null)
        return "Null";
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
    else if (rendererBackendType == RendererBackendType::Null)
        return "Null";
    else
        return "Unknown renderer backend";
}

std::string GetBackendDescription()
{
    const auto& params = GetRunnerParams();
    #ifdef HELLOIMGUI_WITH_REMOTE_DISPLAY
    if (params->remoteParams.enableRemoting)
        return "Remote";
    #endif
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

void ChangeWindowSize(const ScreenSize &windowSize)
{
    gLastRunner->ChangeWindowSize(windowSize);
}


void UseWindowFullMonitorWorkArea()
{
    
    gLastRunner->UseWindowFullMonitorWorkArea();
}


bool ShouldRemoteDisplay()
{
    return gLastRunner->ShouldRemoteDisplay();
}


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