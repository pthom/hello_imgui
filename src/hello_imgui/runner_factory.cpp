#include "runner_factory.h"

#include "backend_impls/runner_emscripten.h"
#include "backend_impls/runner_glfw.h"
#include "backend_impls/runner_sdl.h"

namespace HelloImGui
{
#ifdef HELLOIMGUI_USE_GLFW
std::unique_ptr<AbstractRunner> FactorRunnerGlfw() { return std::make_unique<RunnerGlfw>(); }
#endif

#ifdef HELLOIMGUI_USE_SDL
std::unique_ptr<AbstractRunner> FactorRunnerSdl() { return std::make_unique<RunnerSdl>(); }
#endif

#ifdef __EMSCRIPTEN__
std::unique_ptr<AbstractRunner> FactorRunnerEmscripten() { return std::make_unique<RunnerEmscripten>(); }
#endif

std::unique_ptr<AbstractRunner> FactorRunner()
{
#ifdef HELLOIMGUI_USE_GLFW
    return FactorRunnerGlfw();
#endif
#ifdef HELLOIMGUI_USE_SDL
    return FactorRunnerSdl();
#endif
#ifdef __EMSCRIPTEN__
    return FactorRunnerEmscripten();
#endif
}
}  // namespace HelloImGui
