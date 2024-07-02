#include "hello_imgui/hello_imgui_theme.h"
#include "hello_imgui/hello_imgui.h"

namespace HelloImGui
{

    void ShowThemeTweakGuiWindow(bool* p_open)
    {
        bool showWindow = true;
        if (p_open != nullptr)
            showWindow = *p_open;

        if (showWindow)
        {
            auto& tweakedTheme = HelloImGui::GetRunnerParams()->imGuiWindowParams.tweakedTheme;
            ImGui::SetNextWindowSize(HelloImGui::EmToVec2(20.f, 46.f), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Theme Tweaks", p_open))
            {
                if (ImGuiTheme::ShowThemeTweakGui(&tweakedTheme))
                    ApplyTweakedTheme(tweakedTheme);
            }
            ImGui::End();
        }
    }

} // namespace HellImGui
