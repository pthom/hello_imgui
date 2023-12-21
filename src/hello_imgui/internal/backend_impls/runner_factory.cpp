#include "hello_imgui/internal/backend_impls/runner_factory.h"

#include "hello_imgui/internal/backend_impls/runner_emscripten.h"
#include "hello_imgui/internal/backend_impls/runner_glfw3.h"
#include "hello_imgui/internal/backend_impls/runner_sdl2.h"


namespace HelloImGui
{
#ifdef HELLOIMGUI_USE_GLFW3
std::unique_ptr<AbstractRunner> FactorRunnerGlfw3(RunnerParams& params)
{
    return std::make_unique<RunnerGlfw3>(params);
}
#endif

#ifdef HELLOIMGUI_USE_SDL2
std::unique_ptr<AbstractRunner> FactorRunnerSdl2(RunnerParams& params)
{
    return std::make_unique<RunnerSdl2>(params);
}
#endif

#ifdef __EMSCRIPTEN__
std::unique_ptr<AbstractRunner> FactorRunnerEmscripten(RunnerParams & params)
{
    return std::make_unique<RunnerEmscripten>(params);
}
#endif

std::unique_ptr<AbstractRunner> FactorRunner(RunnerParams& params)
{
    if (params.backendType == BackendType::FirstAvailable)
    {
#ifdef __EMSCRIPTEN__
        return FactorRunnerEmscripten(params);
#endif
#ifdef HELLOIMGUI_USE_GLFW3
        return FactorRunnerGlfw3(params);
#endif
#ifdef HELLOIMGUI_USE_SDL2
        return FactorRunnerSdl2(params);
#endif
        IM_ASSERT(false); // HelloImGui::FactorRunner no backend selected!"
        return nullptr;
    }
    else
    {
        if (params.backendType == BackendType::Sdl)
        {
#ifdef HELLOIMGUI_USE_SDL2
            return FactorRunnerSdl2(params);
#else
            IM_ASSERT(false); //Sdl backend is not available!
#endif
        }
        else if(params.backendType == BackendType::Glfw)
        {
#ifdef HELLOIMGUI_USE_GLFW3
            return FactorRunnerGlfw3(params);
#else
            IM_ASSERT(false); //Glfw backend is not available!
#endif
        }
        else
        {
            IM_ASSERT(false); //Bad backend type!
        }
        return nullptr;
    }
}

}  // namespace HelloImGui
