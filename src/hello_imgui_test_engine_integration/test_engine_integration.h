#include "hello_imgui/runner_params.h"


namespace HelloImGui
{
    namespace TestEngineCallbacks
    {
        void Setup();
        void PostSwap();
        void TearDown_ImGuiContextAlive();
        void TearDown_ImGuiContextDestroyed();

        bool IsRunningTest();
    } // namespace TestEngineCallbacks
}
