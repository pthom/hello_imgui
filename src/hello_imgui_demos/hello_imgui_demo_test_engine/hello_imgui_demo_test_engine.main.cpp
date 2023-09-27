#include "hello_imgui/hello_imgui.h"

#ifdef HELLOIMGUI_WITH_TEST_ENGINE

#include "imgui_test_engine/imgui_te_context.h"
#include "imgui_test_engine/imgui_te_ui.h"
#include "imgui_test_engine/imgui_capture_tool.h"
#include "hello_imgui_test_engine_integration/test_engine_integration.h"


void RegisterAppMinimalTests(ImGuiTestEngine* e)
{
    ImGuiTest* t = NULL;

    //-----------------------------------------------------------------
    // ## Demo Test: Use variables to communicate data between GuiFunc and TestFunc
    //-----------------------------------------------------------------

    t = IM_REGISTER_TEST(e, "demo_tests", "test2");
    struct TestVars2 { int MyInt = 42; };
    t->SetVarsDataType<TestVars2>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        TestVars2& vars = ctx->GetVars<TestVars2>();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::SliderInt("Slider", &vars.MyInt, 0, 1000);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        TestVars2& vars = ctx->GetVars<TestVars2>();
        ctx->SetRef("Test Window");

        IM_CHECK_EQ(vars.MyInt, 42);
        ctx->ItemInputValue("Slider", 123);
        IM_CHECK_EQ(vars.MyInt, 123);
    };

    //-----------------------------------------------------------------
    // ## Open Metrics window
    //-----------------------------------------------------------------

    t = IM_REGISTER_TEST(e, "demo_tests", "open_metrics");
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Dear ImGui Demo");
        ctx->MenuCheck("Tools/Metrics\\/Debugger");
    };

    //-----------------------------------------------------------------
    // ## Capture entire Dear ImGui Demo window.
    //-----------------------------------------------------------------

    t = IM_REGISTER_TEST(e, "demo_tests", "capture_screenshot");
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Dear ImGui Demo");
        ctx->ItemOpen("Widgets");       // Open collapsing header
        ctx->ItemOpenAll("Basic");      // Open tree node and all its descendant
        ctx->CaptureScreenshotWindow("Dear ImGui Demo", ImGuiCaptureFlags_StitchAll | ImGuiCaptureFlags_HideMouseCursor);
    };

    t = IM_REGISTER_TEST(e, "demo_tests", "capture_video");
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Dear ImGui Demo");
        ctx->ItemCloseAll("");
        ctx->MouseTeleportToPos(ctx->GetWindowByRef("")->Pos);

        ctx->CaptureAddWindow("Dear ImGui Demo"); // Optional: Capture single window
        ctx->CaptureBeginVideo();
        ctx->ItemOpen("Widgets");
        ctx->ItemInputValue("Basic/input text", "My first video!");
        ctx->CaptureEndVideo();
    };


}

#endif


void Gui()
{
    ImGui::Text("Hello");
    //ImGui::ShowDemoWindow();
    ImGuiTestEngine_ShowTestEngineWindows(HelloImGui::GetImGuiTestEngine(), NULL);

}


int main(int, char *[])
{
    HelloImGui::RunnerParams runnerParams;
    runnerParams.callbacks.ShowGui = Gui;

#ifdef HELLOIMGUI_WITH_TEST_ENGINE
    runnerParams.callbacks.PostInit = [](){
        RegisterAppMinimalTests(HelloImGui::GetImGuiTestEngine());
    };
#endif

    HelloImGui::Run(runnerParams);
    return 0;
}
