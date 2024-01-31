#ifdef HELLOIMGUI_WITH_TEST_ENGINE

#include "hello_imgui/hello_imgui.h"

#include "imgui_test_engine/imgui_te_engine.h"
#include "imgui_test_engine/imgui_te_context.h"
#include "imgui_test_engine/imgui_te_ui.h"
#include "hello_imgui_test_engine_integration/test_engine_integration.h"


void MyRegisterTests()
{
    ImGuiTestEngine* engine = HelloImGui::GetImGuiTestEngine();

    ImGuiTest* t = NULL;

    // ## Open Metrics window
    t = IM_REGISTER_TEST(engine, "demo_tests", "open_metrics");
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Dear ImGui Demo");
        ctx->MenuCheck("Tools/Metrics\\/Debugger");
    };

    // ## Capture entire Dear ImGui Demo window.
    t = IM_REGISTER_TEST(engine, "demo_tests", "capture_screenshot");
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Dear ImGui Demo");
        ctx->ItemOpen("Widgets");       // Open collapsing header
        ctx->ItemOpenAll("Basic");      // Open tree node and all its descendant
        ctx->CaptureScreenshotWindow("Dear ImGui Demo", ImGuiCaptureFlags_StitchAll | ImGuiCaptureFlags_HideMouseCursor);
    };
}


void AppGui()
{
    ImGui::Text("Hello");
    ImGui::ShowDemoWindow();
    ImGuiTestEngine_ShowTestEngineWindows(HelloImGui::GetImGuiTestEngine(), NULL);

}


int main(int, char *[])
{
    HelloImGui::RunnerParams runnerParams;
    runnerParams.useImGuiTestEngine = true;

    runnerParams.callbacks.ShowGui = AppGui;
    runnerParams.callbacks.RegisterTests = MyRegisterTests;

    HelloImGui::Run(runnerParams);
    return 0;
}

#else // #ifdef HELLOIMGUI_WITH_TEST_ENGINE
#include <stdio.h>
int main(int, char *[]) { fprintf(stderr, "Please compile HelloImGui with the option -DHELLOIMGUI_WITH_TEST_ENGINE=ON");}
#endif // #ifdef HELLOIMGUI_WITH_TEST_ENGINE
