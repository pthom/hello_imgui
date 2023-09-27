#include "hello_imgui/runner_params.h"

struct ImGuiTestEngine;

namespace HelloImGui
{
    namespace TestEngineCallbacks
    {
        void Setup();
        void PostSwap();
        void TearDown_ImGuiContextAlive();
        void TearDown_ImGuiContextDestroyed();
    } // namespace TestEngineCallbacks

    ImGuiTestEngine* GetImGuiTestEngine();
}
