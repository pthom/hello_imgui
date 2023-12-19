#include "imgui_test_engine/imgui_te_engine.h"
#include "hello_imgui/runner_params.h"
#include "hello_imgui/internal/functional_utils.h"
#include "hello_imgui/internal/backend_impls/opengl_setup_helper/opengl_screenshot.h"

namespace HelloImGui
{
    ImGuiTestEngine *GHImGuiTestEngine = nullptr;

    namespace TestEngineCallbacks
    {

        void _SetOptions()
        {
            ImGuiTestEngineIO& test_io = ImGuiTestEngine_GetIO(GHImGuiTestEngine);
            test_io.ConfigVerboseLevel = ImGuiTestVerboseLevel_Info;
            test_io.ConfigVerboseLevelOnError = ImGuiTestVerboseLevel_Debug;
            test_io.ConfigRunSpeed = ImGuiTestRunSpeed_Normal; // Default to slowest mode in this demo

#ifdef HELLOIMGUI_HAS_OPENGL
            test_io.ScreenCaptureFunc = HelloImGui::ImGuiApp_ImplGL_CaptureFramebuffer;
#endif
        }

        void Setup()
        {
            // Setup test engine
            GHImGuiTestEngine = ImGuiTestEngine_CreateContext();

            _SetOptions();

            // Start test engine
            ImGuiTestEngine_Start(GHImGuiTestEngine, ImGui::GetCurrentContext());
            ImGuiTestEngine_InstallDefaultCrashHandler();
        }

        void PostSwap()
        {
            // Call after your rendering. This is mostly to support screen/video capturing features.
            ImGuiTestEngine_PostSwap(GHImGuiTestEngine);
        }

        void TearDown_ImGuiContextAlive()
        {
            ImGuiTestEngine_Stop(GHImGuiTestEngine);
        }
        void TearDown_ImGuiContextDestroyed()
        {
            // IMPORTANT: we need to destroy the Dear ImGui context BEFORE the test engine context, so .ini data may be saved.
            ImGuiTestEngine_DestroyContext(GHImGuiTestEngine);
        }

        bool IsRunningTest()
        {
            bool r = !ImGuiTestEngine_IsTestQueueEmpty(GHImGuiTestEngine);
            return r;
        }

    } // namespace TestEngineCallbacks
}