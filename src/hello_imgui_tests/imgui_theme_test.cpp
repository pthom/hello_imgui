#include "doctest.h"
#include "hello_imgui/imgui_theme.h"
#include "imgui.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>


namespace
{
    // fg (with its alpha) drawn over an opaque bg
    ImVec4 Over(ImVec4 fg, ImVec4 bg)
    {
        auto mix = [&](float f, float b) { return std::min(std::max(f, 0.f), 1.f) * fg.w + b * (1.f - fg.w); };
        return ImVec4(mix(fg.x, bg.x), mix(fg.y, bg.y), mix(fg.z, bg.z), 1.f);
    }

    float Luminance(ImVec4 c)
    {
        auto lin = [](float v) { return v <= 0.03928f ? v / 12.92f : std::pow((v + 0.055f) / 1.055f, 2.4f); };
        return 0.2126f * lin(c.x) + 0.7152f * lin(c.y) + 0.0722f * lin(c.z);
    }

    // The WCAG contrast ratio of two opaque colors: 1 (none) to 21 (black on white)
    float Contrast(ImVec4 a, ImVec4 b)
    {
        float la = Luminance(a), lb = Luminance(b);
        return (std::max(la, lb) + 0.05f) / (std::min(la, lb) + 0.05f);
    }
}


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


// Regression test: in the light themes, the text cursor was white on white, and the links near white
// (the colors added to Dear ImGui after a theme was written came from the current style).
TEST_CASE("Every theme is readable: the caret, the links, the slider grab, the selection")
{
    ImGuiContext* ctx = ImGui::CreateContext();
    for (int i = 0; i < ImGuiTheme::ImGuiTheme_Count; ++i)
    {
        auto theme = (ImGuiTheme::ImGuiTheme_)i;
        std::string name = ImGuiTheme::ImGuiTheme_Name(theme);
        CAPTURE(name);
        ImGuiStyle style = ImGuiTheme::ThemeToStyle(theme);
        const ImVec4* colors = style.Colors;
        bool light = Luminance(colors[ImGuiCol_WindowBg]) > 0.4f;
        ImVec4 window = Over(colors[ImGuiCol_WindowBg], light ? ImVec4(1, 1, 1, 1) : ImVec4(0, 0, 0, 1));
        ImVec4 frame = Over(colors[ImGuiCol_FrameBg], window);
        CHECK(Contrast(Over(colors[ImGuiCol_InputTextCursor], frame), frame) > 3.f);
        CHECK(Contrast(Over(colors[ImGuiCol_TextLink], window), window) > 1.8f);  // Dear ImGui's Light: 2.6

        // Hello ImGui's own themes (Dear ImGui's Classic has a selection at 1.0). The slider grab was invisible in
        // SoDark, BlackIsBlack and Cherry; the disabled text in Darcula; the selection in Cherry, SoDark, MaterialFlat
        if (theme > ImGuiTheme::ImGuiTheme_ImGuiColorsLight)
        {
            CHECK(Contrast(Over(colors[ImGuiCol_SliderGrab], frame), frame) > 1.5f);       // Dear ImGui's Dark: 3.6
            CHECK(Contrast(Over(colors[ImGuiCol_TextSelectedBg], frame), frame) > 1.35f);  // Dark: 1.7, Light: 1.4
            CHECK(Contrast(Over(colors[ImGuiCol_TextDisabled], window), window) > 2.2f);   // Light: 2.5
            CHECK(Contrast(Over(colors[ImGuiCol_PlotHistogram], frame), frame) > 1.5f);    // Light: 1.95
        }
    }
    ImGui::DestroyContext(ctx);
}

TEST_CASE("A theme's colors do not depend on the current style")
{
    ImGuiContext* ctx = ImGui::CreateContext();
    for (int i = 0; i < ImGuiTheme::ImGuiTheme_Count; ++i)
    {
        auto theme = (ImGuiTheme::ImGuiTheme_)i;
        std::string name = ImGuiTheme::ImGuiTheme_Name(theme);
        CAPTURE(name);
        ImGui::StyleColorsDark();
        ImGuiStyle after_dark = ImGuiTheme::ThemeToStyle(theme);
        ImGui::StyleColorsLight();
        ImGuiStyle after_light = ImGuiTheme::ThemeToStyle(theme);
        CHECK(std::memcmp(after_dark.Colors, after_light.Colors, sizeof(after_dark.Colors)) == 0);
    }
    ImGui::DestroyContext(ctx);
}
