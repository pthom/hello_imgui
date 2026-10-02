#include "hello_imgui/runner_params.h"

struct ImGuiTestCoroutineInterface;


namespace HelloImGui
{
    namespace TestEngineCallbacks
    {
        // Replaces the test engine's std::thread coroutine (e.g. where there are no threads). Call it before Setup().
        void SetCoroutineInterface(ImGuiTestCoroutineInterface* coroutineInterface);

        void Setup();
        void PostSwap();
        void TearDown_ImGuiContextAlive();
        void TearDown_ImGuiContextDestroyed();

        bool IsRunningTest();
    } // namespace TestEngineCallbacks
}
