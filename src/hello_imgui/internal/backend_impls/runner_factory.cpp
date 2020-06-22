#include "hello_imgui/internal/backend_impls/runner_factory.h"

#include "hello_imgui/internal/backend_impls/runner_emscripten.h"
#include "hello_imgui/internal/backend_impls/runner_glfw_opengl3.h"
#include "hello_imgui/internal/backend_impls/runner_qt.h"
#include "hello_imgui/internal/backend_impls/runner_sdl_opengl3.h"

#include <stdexcept>

namespace HelloImGui
{
#ifdef HELLOIMGUI_USE_GLFW_OPENGL3
std::unique_ptr<AbstractRunner> FactorRunnerGlfwOpenGl3(RunnerParams& params)
{
    return std::make_unique<RunnerGlfwOpenGl3>(params);
}
#endif

#ifdef HELLOIMGUI_USE_SDL_OPENGL3
std::unique_ptr<AbstractRunner> FactorRunnerSdlOpenGl3(RunnerParams& params)
{
    return std::make_unique<RunnerSdlOpenGl3>(params);
}
#endif

#ifdef HELLOIMGUI_USE_QT
std::unique_ptr<AbstractRunner> FactorRunnerQt(RunnerParams & params)
{
    return std::make_unique<RunnerQt>(params);
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
#ifdef __EMSCRIPTEN__
    return FactorRunnerEmscripten(params);
#endif
#ifdef HELLOIMGUI_USE_SDL_OPENGL3
    return FactorRunnerSdlOpenGl3(params);
#endif
#ifdef HELLOIMGUI_USE_GLFW_OPENGL3
    return FactorRunnerGlfwOpenGl3(params);
#endif
#ifdef HELLOIMGUI_USE_QT
    return FactorRunnerQt(params);
#endif
    throw std::logic_error("HelloImGui::FactorRunner no bakend selected!");
}

}  // namespace HelloImGui
