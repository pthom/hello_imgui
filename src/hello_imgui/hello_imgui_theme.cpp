#include "hello_imgui/hello_imgui_theme.h"

namespace HelloImGui
{
    bool gShowTweakWindow = false;

    void Theme_WindowGui(ImGuiTheme::ImGuiTweakedTheme& tweakedTheme)
    {
        if (gShowTweakWindow)
        {
            ImGui::SetNextWindowSize(ImVec2(600.f, 600.f), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Theme Tweaks", &gShowTweakWindow))
            {
                if (ImGuiTheme::ShowThemeTweakGui(&tweakedTheme))
                {
                    ApplyTweakedTheme(tweakedTheme);
                }
            }
            ImGui::End();
        }
    }

    void Theme_MenuGui(ImGuiTheme::ImGuiTweakedTheme& tweakedTheme)
    {
        if (ImGui::BeginMenu("Theme"))
        {
            if (ImGui::MenuItem("Theme tweak window", nullptr, gShowTweakWindow))
                gShowTweakWindow = !gShowTweakWindow;
            ImGui::Separator();
            for (int i = 0; i < ImGuiTheme::ImGuiTheme_Count; ++i)
            {
                ImGuiTheme::ImGuiTheme_ theme = (ImGuiTheme::ImGuiTheme_)(i);
                bool selected = (theme == tweakedTheme.Theme);
                if (ImGui::MenuItem(ImGuiTheme::ImGuiTheme_Name(theme), nullptr, selected))
                {
                    tweakedTheme.Theme = theme;
                    ImGuiTheme::ApplyTheme(theme);
                }
            }
            ImGui::EndMenu();
        }
    }
} // namespace HellImGui
