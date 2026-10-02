#include "doctest.h"
#include "hello_imgui/imgui_theme.h"
#include "imgui.h"


// ImGuiTheme is usable standalone: an ImGui context, no HelloImGui::Run().
// Regression test: from v1.92.6 to v1.92.900, ApplyTheme threw
// "HelloImGui::GetRunnerParams() would return null. Did you call HelloImGui::Run()?"
TEST_CASE("ImGuiTheme::ApplyTheme works without HelloImGui::Run()")
{
    ImGuiContext* ctx = ImGui::CreateContext();
    CHECK_NOTHROW(ImGuiTheme::ApplyTheme(ImGuiTheme::ImGuiTheme_ImGuiColorsDark));
    CHECK_NOTHROW(ImGuiTheme::ApplyTheme(ImGuiTheme::ImGuiTheme_DarculaDarker));

    ImGuiTheme::ImGuiTweakedTheme tweaked;
    tweaked.Theme = ImGuiTheme::ImGuiTheme_PhotoshopStyle;
    CHECK_NOTHROW(ImGuiTheme::ApplyTweakedTheme(tweaked));

    ImGui::DestroyContext(ctx);
}
