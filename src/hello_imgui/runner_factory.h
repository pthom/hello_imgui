#pragma once
#include "hello_imgui/abstract_runner.h"
#include <memory>

namespace HelloImGui
{
#ifdef HELLOIMGUI_USE_GLFW
std::unique_ptr<AbstractRunner> FactorRunnerGlfw();
#endif

#ifdef HELLOIMGUI_USE_SDL
std::unique_ptr<AbstractRunner> FactorRunnerSdl();
#endif

#ifdef __EMSCRIPTEN__
std::unique_ptr<AbstractRunner> FactorRunnerEmscripten();
#endif

std::unique_ptr<AbstractRunner> FactorRunner();
}  // namespace HelloImGui
