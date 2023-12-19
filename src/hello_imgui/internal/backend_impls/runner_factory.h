#pragma once
#include "hello_imgui/internal/backend_impls/abstract_runner.h"
#include <memory>

namespace HelloImGui
{
    std::unique_ptr<AbstractRunner> FactorRunner(RunnerParams & params);
}  // namespace HelloImGui
