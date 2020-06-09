#pragma once
#include "hello_imgui/internal/backend_impls/abstract_runner.h"
#include <memory>

namespace HelloImGui
{
#ifdef HELLOIMGUI_USE_GLFW_OPENGL3
std::unique_ptr<AbstractRunner> FactorRunnerGlfwOpenGl3(RunnerParams & params);
#endif

#ifdef HELLOIMGUI_USE_SDL_OPENGL3
std::unique_ptr<AbstractRunner> FactorRunnerSdlOpenGl3(RunnerParams & params);
#endif

#ifdef HELLOIMGUI_USE_QT
std::unique_ptr<AbstractRunner> FactorRunnerQt(RunnerParams & params);
#endif

#ifdef __EMSCRIPTEN__
std::unique_ptr<AbstractRunner> FactorRunnerEmscripten(RunnerParams & params);
#endif

std::unique_ptr<AbstractRunner> FactorRunner(RunnerParams & params);
}  // namespace HelloImGui
