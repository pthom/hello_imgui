#include "doctest.h"
#include "hello_imgui/imgui_theme.h"
#include "imgui.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>
#include <tuple>


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

    // How different two opaque colors look: CIE76 delta E (2.3 is barely visible; Dear ImGui Dark's hovers: 11 to 40)
    float DeltaE(ImVec4 a, ImVec4 b)
    {
        auto lab = [](ImVec4 c, float out[3]) {
            auto lin = [](float v) { return v <= 0.04045f ? v / 12.92f : std::pow((v + 0.055f) / 1.055f, 2.4f); };
            float r = lin(c.x), g = lin(c.y), b = lin(c.z);
            float x = (0.4124f * r + 0.3576f * g + 0.1805f * b) / 0.95047f;
            float y = 0.2126f * r + 0.7152f * g + 0.0722f * b;
            float z = (0.0193f * r + 0.1192f * g + 0.9505f * b) / 1.08883f;
            auto f = [](float t) { return t > 0.008856f ? std::cbrt(t) : 7.787f * t + 16.f / 116.f; };
            out[0] = 116.f * f(y) - 16.f;
            out[1] = 500.f * (f(x) - f(y));
            out[2] = 200.f * (f(y) - f(z));
        };
        float la[3], lb[3];
        lab(a, la);
        lab(b, lb);
        return std::sqrt((la[0] - lb[0]) * (la[0] - lb[0]) + (la[1] - lb[1]) * (la[1] - lb[1]) +
                         (la[2] - lb[2]) * (la[2] - lb[2]));
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
TEST_CASE("Every theme is readable: the caret, links, selection, slider grab, borders, tabs...")
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

            // The structure and the widgets (they faded into the window in SoDark, BlackIsBlack, MaterialFlat, Cherry,
            // Darcula, PhotoshopStyle, GrayVariations_Darker)
            auto on_window = [&](ImGuiCol col) { return Contrast(Over(colors[col], window), window); };
            CHECK(on_window(ImGuiCol_TableBorderStrong) > 1.8f);  // Dear ImGui's Dark: 2.4
            CHECK(on_window(ImGuiCol_TableBorderLight) > 1.3f);
            CHECK(on_window(ImGuiCol_Separator) > 1.5f);          // Dark: 1.8
            CHECK(on_window(ImGuiCol_Border) > 1.5f);
            CHECK(on_window(ImGuiCol_Button) > 1.2f);             // Dark: 2.0, Light: 1.45
            CHECK(on_window(ImGuiCol_Header) > 1.2f);
            ImVec4 scrollbar = Over(colors[ImGuiCol_ScrollbarBg], window);
            CHECK(Contrast(Over(colors[ImGuiCol_ScrollbarGrab], scrollbar), scrollbar) > 1.45f);  // Dark: 2.4
            ImVec4 tab_selected = Over(colors[ImGuiCol_TabSelected], window);
            CHECK(Contrast(Over(colors[ImGuiCol_Text], tab_selected), tab_selected) > 3.3f);     // the tab's label
            float h, s_selected, s_tab, v;
            ImGui::ColorConvertRGBtoHSV(colors[ImGuiCol_TabSelected].x, colors[ImGuiCol_TabSelected].y,
                                        colors[ImGuiCol_TabSelected].z, h, s_selected, v);
            ImGui::ColorConvertRGBtoHSV(colors[ImGuiCol_Tab].x, colors[ImGuiCol_Tab].y, colors[ImGuiCol_Tab].z,
                                        h, s_tab, v);
            if (s_selected < 0.2f && s_tab < 0.2f)  // gray tabs: the selected one must differ by its brightness
                CHECK(Contrast(tab_selected, Over(colors[ImGuiCol_Tab], window)) > 1.4f);
        }
    }
    ImGui::DestroyContext(ctx);
}

// Regression test: hovers that did not show (the tabs and scrollbars of the light grays, Photoshop's button), text
// on Darcula's blue hover at 1.9, scrollbar grabs fainter when hovered, Material Flat's check mark gone while held.
TEST_CASE("Every theme's states show: hover, press, the grab at rest, the check mark")
{
    ImGuiContext* ctx = ImGui::CreateContext();
    for (int i = ImGuiTheme::ImGuiTheme_ImGuiColorsLight + 1; i < ImGuiTheme::ImGuiTheme_Count; ++i)
    {
        auto theme = (ImGuiTheme::ImGuiTheme_)i;
        std::string name = ImGuiTheme::ImGuiTheme_Name(theme);
        CAPTURE(name);
        ImGuiStyle style = ImGuiTheme::ThemeToStyle(theme);
        const ImVec4* colors = style.Colors;
        bool light = Luminance(colors[ImGuiCol_WindowBg]) > 0.4f;
        ImVec4 window = Over(colors[ImGuiCol_WindowBg], light ? ImVec4(1, 1, 1, 1) : ImVec4(0, 0, 0, 1));
        auto on = [&](ImGuiCol col, ImVec4 bg) { return Over(colors[col], bg); };
        auto on_window = [&](ImGuiCol col) { return on(col, window); };

        // A hover shows (delta E 10), a press shows (5)
        for (auto [rest, hovered, active] : {std::tuple{ImGuiCol_Button, ImGuiCol_ButtonHovered, ImGuiCol_ButtonActive},
                                             std::tuple{ImGuiCol_FrameBg, ImGuiCol_FrameBgHovered, ImGuiCol_FrameBgActive},
                                             std::tuple{ImGuiCol_Header, ImGuiCol_HeaderHovered, ImGuiCol_HeaderActive}})
        {
            std::string state = ImGui::GetStyleColorName(hovered);
            CAPTURE(state);
            CHECK(DeltaE(on_window(rest), on_window(hovered)) > 10.f);
            CHECK(DeltaE(on_window(hovered), on_window(active)) > 5.f);
        }
        CHECK(DeltaE(on_window(ImGuiCol_Tab), on_window(ImGuiCol_TabHovered)) > 10.f);

        // Text reads on a hovered or pressed item (3:1)
        for (ImGuiCol bg : {ImGuiCol_ButtonHovered, ImGuiCol_ButtonActive, ImGuiCol_FrameBgHovered,
                            ImGuiCol_FrameBgActive, ImGuiCol_HeaderHovered, ImGuiCol_HeaderActive, ImGuiCol_TabHovered})
        {
            std::string background = ImGui::GetStyleColorName(bg);
            CAPTURE(background);
            ImVec4 b = on_window(bg);
            CHECK(Contrast(on(ImGuiCol_Text, b), b) > 3.f);
        }

        // The slider grab at rest, the check mark in every state, the held grab (3:1); the digits over the grab (2:1)
        ImVec4 frame = on_window(ImGuiCol_FrameBg);
        ImVec4 grab = on(ImGuiCol_SliderGrab, frame);
        CHECK(Contrast(grab, frame) > 3.f);
        CHECK(Contrast(on(ImGuiCol_Text, grab), grab) > 2.f);
        for (ImGuiCol bg : {ImGuiCol_FrameBg, ImGuiCol_FrameBgHovered, ImGuiCol_FrameBgActive})
        {
            std::string frame_state = ImGui::GetStyleColorName(bg);
            CAPTURE(frame_state);
            ImVec4 b = on_window(bg);
            CHECK(Contrast(on(ImGuiCol_CheckMark, b), b) > 3.f);
        }
        ImVec4 frame_active = on_window(ImGuiCol_FrameBgActive);
        CHECK(Contrast(on(ImGuiCol_SliderGrabActive, frame_active), frame_active) > 3.f);

        // The scrollbar grab: its hover shows, and it never gets fainter when hovered or held
        ImVec4 track = on_window(ImGuiCol_ScrollbarBg);
        auto grab_on_track = [&](ImGuiCol col) { return Contrast(on(col, track), track); };
        CHECK(DeltaE(on(ImGuiCol_ScrollbarGrab, track), on(ImGuiCol_ScrollbarGrabHovered, track)) > 10.f);
        CHECK(DeltaE(on(ImGuiCol_ScrollbarGrabHovered, track), on(ImGuiCol_ScrollbarGrabActive, track)) > 5.f);
        CHECK(grab_on_track(ImGuiCol_ScrollbarGrabHovered) >= grab_on_track(ImGuiCol_ScrollbarGrab) - 0.01f);
        CHECK(grab_on_track(ImGuiCol_ScrollbarGrabActive) >= grab_on_track(ImGuiCol_ScrollbarGrabHovered) - 0.01f);
    }
    ImGui::DestroyContext(ctx);
}

// Regression test: the tweaks' multipliers pushed some components above 1 (WhiteIsWhite's WindowBg was 6.64)
TEST_CASE("Every theme's colors stay in [0, 1], tweaked ones too")
{
    ImGuiContext* ctx = ImGui::CreateContext();
    auto check_in_range = [](const ImGuiStyle& style) {
        for (int c = 0; c < ImGuiCol_COUNT; ++c)
        {
            std::string color_name = ImGui::GetStyleColorName(c);
            CAPTURE(color_name);
            const ImVec4& col = style.Colors[c];
            for (float f : {col.x, col.y, col.z, col.w})
                CHECK((f >= 0.f && f <= 1.f));
        }
    };
    for (int i = 0; i < ImGuiTheme::ImGuiTheme_Count; ++i)
    {
        auto theme = (ImGuiTheme::ImGuiTheme_)i;
        std::string name = ImGuiTheme::ImGuiTheme_Name(theme);
        CAPTURE(name);
        check_in_range(ImGuiTheme::ThemeToStyle(theme));

        ImGuiTheme::ImGuiTweakedTheme tweaked;
        tweaked.Theme = theme;
        tweaked.Tweaks.Hue = 0.3f;
        tweaked.Tweaks.SaturationMultiplier = 3.f;
        tweaked.Tweaks.ValueMultiplierFront = 3.f;
        tweaked.Tweaks.ValueMultiplierBg = 3.f;
        tweaked.Tweaks.ValueMultiplierText = 3.f;
        tweaked.Tweaks.ValueMultiplierFrameBg = 3.f;
        check_in_range(ImGuiTheme::TweakedThemeThemeToStyle(tweaked));
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
