//
// Theme tweak utilities for ImGui.
// Reuse and adaptation of imgui_theme.h and imgui_theme.cpp file is granted for other projects
// provided the origin of those files is stated in the copied version.
// Some themes were adapted by themes posted by ImGui users at https://github.com/ocornut/imgui/issues/707
//
#include "hello_imgui/imgui_theme.h"
#include <string>
#include <stack>

namespace ImGuiTheme
{
    namespace ThemeTweakImpl
    {
        void ApplyRounding(ImGuiStyle& io_style, float rounding, float scrollbar_ratio)
        {
            io_style.WindowRounding = rounding;
            io_style.ChildRounding = rounding;
            io_style.PopupRounding = rounding;
            io_style.FrameRounding = rounding;
            io_style.GrabRounding = rounding;
            io_style.TabRounding = rounding;
            io_style.ScrollbarRounding = rounding * scrollbar_ratio;
        }

        void ApplyAlphaMultiplier(ImGuiStyle& io_style, float alpha_multiplier, ImGuiStyle& reference_style)
        {
            for (int i = 0; i < ImGuiCol_COUNT; ++i)
            {
                ImGuiCol_ colEnum = (ImGuiCol_)i;
                ImVec4& col = io_style.Colors[i];
                const ImVec4& col_ref = reference_style.Colors[i];
                bool apply = false;
                {
                    bool isWindowBackground =
                        (colEnum == ImGuiCol_FrameBg) || (colEnum == ImGuiCol_WindowBg ) || (colEnum == ImGuiCol_ChildBg);
                    bool isAlreadyTransparent = col.w < 1.00f;
                    bool isDimColor = (colEnum == ImGuiCol_ModalWindowDimBg) || (colEnum == ImGuiCol_NavWindowingDimBg);
                    if (! isDimColor && ( isAlreadyTransparent ||  isWindowBackground) )
                        apply = true;
                }
                if (apply)
                {
                    col.w = alpha_multiplier * col_ref.w;
                    if (col.w < 0.)
                        col.w = 0.;
                    if (col.w > 1.f)
                        col.w = 1.f;
                }
            }
        }

        void ApplySaturationMultiplier(ImGuiStyle& io_style, float saturation_multiplier, ImGuiStyle& reference_style)
        {
            for (int i = 0; i < ImGuiCol_COUNT; ++i)
            {
                ImVec4 &col = io_style.Colors[i];
                const ImVec4 &col_ref = reference_style.Colors[i];

                float h, s, v;
                ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, h, s, v);
                float h_ref, s_ref, v_ref;
                ImGui::ColorConvertRGBtoHSV(col_ref.x, col_ref.y, col_ref.z, h_ref, s_ref, v_ref);
                s = s_ref * saturation_multiplier;
                ImGui::ColorConvertHSVtoRGB(h, s, v, col.x, col.y, col.z);
            }
        }

        ImGuiCol_ gBgColors[] = {
            ImGuiCol_WindowBg,
            ImGuiCol_ChildBg,
            ImGuiCol_PopupBg
        };

        ImGuiCol_ gTextColors[] = {
            ImGuiCol_Text,
            ImGuiCol_TextDisabled,
        };

        enum class ColorCategory
        {
            ColorBg,
            ColorFront,
            ColorText,
            ColorFrameBg
        };

        ColorCategory _GetColorCategory(ImGuiCol_ col)
        {
            if (col == ImGuiCol_FrameBg)
                return ColorCategory::ColorFrameBg;
            for (size_t i = 0; i < IM_ARRAYSIZE(gBgColors); ++i)
                if (col == gBgColors[i])
                    return ColorCategory::ColorBg;
            for (size_t i = 0; i < IM_ARRAYSIZE(gTextColors); ++i)
                if (col == gTextColors[i])
                    return ColorCategory::ColorText;
            return ColorCategory::ColorFront;
        }

        static ImVec4 _ColorValueMultiply(ImVec4 col, float value_multiplier)
        {
            float h, s, v;
            ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, h, s, v);
            v = v * value_multiplier;
            ImVec4 r = col;
            ImGui::ColorConvertHSVtoRGB(h, s, v, r.x, r.y, r.z);
            return r;
        }

        static ImVec4 _ColorSetValue(ImVec4 col, float value)
        {
            float h, s, v;
            ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, h, s, v);
            v = value;
            ImVec4 r = col;
            ImGui::ColorConvertHSVtoRGB(h, s, v, r.x, r.y, r.z);
            return r;
        }

        void _ApplyValueMultiplier(ImGuiStyle& style, float value_multiplier, ImGuiStyle& reference_style, ColorCategory category)
        {
            for (int i = 0; i < ImGuiCol_COUNT; ++i)
            {
                ImGuiCol_ colEnum = (ImGuiCol_) i;
                if (_GetColorCategory(colEnum) == category)
                {
                    ImVec4 &col = style.Colors[i];
                    const ImVec4 &col_ref = reference_style.Colors[i];

                    float h, s, v;
                    ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, h, s, v);
                    float h_ref, s_ref, v_ref;
                    ImGui::ColorConvertRGBtoHSV(col_ref.x, col_ref.y, col_ref.z, h_ref, s_ref, v_ref);
                    v = v_ref * value_multiplier;
                    ImGui::ColorConvertHSVtoRGB(h, s, v, col.x, col.y, col.z);
                }
            }
        }

        void ApplyValueMultiplierFront(ImGuiStyle& style, float value_multiplier, ImGuiStyle& reference_style)
        {
            _ApplyValueMultiplier(style, value_multiplier, reference_style, ColorCategory::ColorFront);
        }

        void ApplyValueMultiplierBg(ImGuiStyle& style, float value_multiplier, ImGuiStyle& reference_style)
        {
            _ApplyValueMultiplier(style, value_multiplier, reference_style, ColorCategory::ColorBg);
        }

        void ApplyValueMultiplierText(ImGuiStyle& style, float value_multiplier, ImGuiStyle& reference_style)
        {
            _ApplyValueMultiplier(style, value_multiplier, reference_style, ColorCategory::ColorText);
        }

        void ApplyValueMultiplierFrameBg(ImGuiStyle& style, float value_multiplier, ImGuiStyle& reference_style)
        {
            _ApplyValueMultiplier(style, value_multiplier, reference_style, ColorCategory::ColorFrameBg);
        }

        void ApplyHue(ImGuiStyle& style, float hue)
        {
            for (int i = 0; i < ImGuiCol_COUNT; i++)
            {
                ImVec4& col = style.Colors[i];

                float h, s, v;
                ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, h, s, v);
                h = hue;
                ImGui::ColorConvertHSVtoRGB(h, s, v, col.x, col.y, col.z);
            }
        }

    }

    namespace ThemesImpl
    {
        ImGuiStyle ImGui_StyleColorsClassic()
        {
            ImGuiStyle style;
            ImGui::StyleColorsClassic(&style);
            return style;
        }

        ImGuiStyle ImGui_StyleColorsDark()
        {
            ImGuiStyle style;
            ImGui::StyleColorsDark(&style);
            return style;
        }

        ImGuiStyle ImGui_StyleColorsLight()
        {
            ImGuiStyle style;
            ImGui::StyleColorsLight(&style);
            return style;
        }

        ImGuiStyle SoDark(float hue)
        {
            ImGuiStyle style;
            ImVec4* colors = style.Colors;
            colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
            colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
            colors[ImGuiCol_WindowBg]               = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
            colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_PopupBg]                = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
            colors[ImGuiCol_Border]                 = ImVec4(0.39f, 0.39f, 0.39f, 0.59f);
            colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
            colors[ImGuiCol_FrameBg]                = ImVec4(0.25f, 0.25f, 0.25f, 0.54f);
            colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
            colors[ImGuiCol_FrameBgActive]          = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
            colors[ImGuiCol_TitleBg]                = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_TitleBgActive]          = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
            colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
            colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
            colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
            colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
            colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
            colors[ImGuiCol_CheckMark]              = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
            colors[ImGuiCol_SliderGrab]             = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
            colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
            colors[ImGuiCol_Button]                 = ImVec4(0.30f, 0.30f, 0.30f, 0.54f);
            colors[ImGuiCol_ButtonHovered]          = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
            colors[ImGuiCol_ButtonActive]           = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
            colors[ImGuiCol_Header]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
            colors[ImGuiCol_HeaderHovered]          = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
            colors[ImGuiCol_HeaderActive]           = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
            colors[ImGuiCol_Separator]              = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
            colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
            colors[ImGuiCol_SeparatorActive]        = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
            colors[ImGuiCol_ResizeGrip]             = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
            colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
            colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
            colors[ImGuiCol_Tab]                    = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
            colors[ImGuiCol_TabHovered]             = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
            colors[ImGuiCol_TabActive]              = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
            colors[ImGuiCol_TabUnfocused]           = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
            colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
            colors[ImGuiCol_DockingPreview]         = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
            colors[ImGuiCol_DockingEmptyBg]         = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_PlotLines]              = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_PlotHistogram]          = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
            colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
            colors[ImGuiCol_TableBorderLight]       = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
            colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
            colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
            colors[ImGuiCol_DragDropTarget]         = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
            colors[ImGuiCol_NavHighlight]           = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
            colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
            colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

            style.WindowPadding                     = ImVec2(8.00f, 8.00f);
            style.FramePadding                      = ImVec2(5.00f, 2.00f);
            style.CellPadding                       = ImVec2(6.00f, 6.00f);
            style.ItemSpacing                       = ImVec2(6.00f, 6.00f);
            style.ItemInnerSpacing                  = ImVec2(6.00f, 6.00f);
            style.TouchExtraPadding                 = ImVec2(0.00f, 0.00f);
            style.IndentSpacing                     = 25;
            style.ScrollbarSize                     = 15;
            style.GrabMinSize                       = 10;
            style.WindowBorderSize                  = 1;
            style.ChildBorderSize                   = 1;
            style.PopupBorderSize                   = 1;
            style.FrameBorderSize                   = 1;
            style.TabBorderSize                     = 1;
            style.WindowRounding                    = 7;
            style.ChildRounding                     = 4;
            style.FrameRounding                     = 3;
            style.PopupRounding                     = 4;
            style.ScrollbarRounding                 = 9;
            style.GrabRounding                      = 3;
            style.LogSliderDeadzone                 = 4;
            style.TabRounding                       = 4;

            ThemeTweakImpl::ApplyHue(style, hue);

            return style;
        }

        ImGuiStyle MaterialFlat()
        {
            ImGuiStyle style;
            // Material Flat style by ImJC1C from ImThemes
            style.Alpha = 1.0f;
            style.DisabledAlpha = 0.5f;
            style.WindowPadding = ImVec2(8.0f, 8.0f);
            style.WindowRounding = 0.0f;
            style.WindowBorderSize = 1.0f;
            style.WindowMinSize = ImVec2(32.0f, 32.0f);
            style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
            style.WindowMenuButtonPosition = ImGuiDir_Left;
            style.ChildRounding = 0.0f;
            style.ChildBorderSize = 1.0f;
            style.PopupRounding = 0.0f;
            style.PopupBorderSize = 1.0f;
            style.FramePadding = ImVec2(4.0f, 3.0f);
            style.FrameRounding = 0.0f;
            style.FrameBorderSize = 0.0f;
            style.ItemSpacing = ImVec2(8.0f, 4.0f);
            style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
            style.CellPadding = ImVec2(4.0f, 2.0f);
            style.IndentSpacing = 21.0f;
            style.ColumnsMinSpacing = 6.0f;
            style.ScrollbarSize = 14.0f;
            style.ScrollbarRounding = 0.0f;
            style.GrabMinSize = 10.0f;
            style.GrabRounding = 0.0f;
            style.TabRounding = 0.0f;
            style.TabBorderSize = 0.0f;
            style.TabMinWidthForCloseButton = 0.0f;
            style.ColorButtonPosition = ImGuiDir_Left;
            style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
            style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

            style.Colors[ImGuiCol_Text] = ImVec4(0.8313725590705872f, 0.8470588326454163f, 0.8784313797950745f, 1.0f);
            style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.8313725590705872f, 0.8470588326454163f, 0.8784313797950745f, 0.501960813999176f);
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1725490242242813f, 0.1921568661928177f, 0.2352941185235977f, 1.0f);
            style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.1587982773780823f);
            style.Colors[ImGuiCol_PopupBg] = ImVec4(0.1725490242242813f, 0.1921568661928177f, 0.2352941185235977f, 1.0f);
            //style.Colors[ImGuiCol_Border] = ImVec4(0.2039215713739395f, 0.2313725501298904f, 0.2823529541492462f, 1.0f);
            style.Colors[ImGuiCol_Border] = ImVec4(60.f / 255.f, 86.f / 255.f, 134.f / 255.f, 1.0f);
            style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            style.Colors[ImGuiCol_FrameBg] = ImVec4(0.105882354080677f, 0.1137254908680916f, 0.1372549086809158f, 0.501960813999176f);
            style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3098039329051971f, 0.6235294342041016f, 0.9333333373069763f, 0.250980406999588f);
            style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.3098039329051971f, 0.6235294342041016f, 0.9333333373069763f, 1.0f);
            style.Colors[ImGuiCol_TitleBg] = ImVec4(0.105882354080677f, 0.1137254908680916f, 0.1372549086809158f, 1.0f);
            style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.105882354080677f, 0.1137254908680916f, 0.1372549086809158f, 1.0f);
            style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.105882354080677f, 0.1137254908680916f, 0.1372549086809158f, 1.0f);
            style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.105882354080677f, 0.1137254908680916f, 0.1372549086809158f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.01960784383118153f, 0.01960784383118153f, 0.01960784383118153f, 0.0f);
            style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.5333333611488342f, 0.5333333611488342f, 0.5333333611488342f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.3333333432674408f, 0.3333333432674408f, 0.3333333432674408f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.6000000238418579f, 0.6000000238418579f, 0.6000000238418579f, 1.0f);
            style.Colors[ImGuiCol_CheckMark] = ImVec4(0.3098039329051971f, 0.6235294342041016f, 0.9333333373069763f, 1.0f);
            style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.239215686917305f, 0.5215686559677124f, 0.8784313797950745f, 1.0f);
            style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9803921580314636f, 1.0f);
            style.Colors[ImGuiCol_Button] = ImVec4(0.3529411822557449f, 0.4125490242242813f, 0.4417647081613541f, 0.501960813999176f);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.1529411822557449f, 0.1725490242242813f, 0.2117647081613541f, 1.0f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.3098039329051971f, 0.6235294342041016f, 0.9333333373069763f, 1.0f);
            style.Colors[ImGuiCol_Header] = ImVec4(0.1529411822557449f * 1.5f, 0.1725490242242813f  * 1.5f, 0.2117647081613541f  * 1.5f, 1.0f);
            style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.3098039329051971f, 0.6235294342041016f, 0.9333333373069763f, 0.250980406999588f);
            style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.3098039329051971f, 0.6235294342041016f, 0.9333333373069763f, 1.0f);
            style.Colors[ImGuiCol_Separator] = ImVec4(0.4274509847164154f, 0.4274509847164154f, 0.4980392158031464f, 0.5f);
            style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.09803921729326248f, 0.4000000059604645f, 0.7490196228027344f, 0.7799999713897705f);
            style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.09803921729326248f, 0.4000000059604645f, 0.7490196228027344f, 1.0f);
            style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.105882354080677f, 0.1137254908680916f, 0.1372549086809158f, 1.0f);
            style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.3098039329051971f, 0.6235294342041016f, 0.9333333373069763f, 0.250980406999588f);
            style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.3098039329051971f, 0.6235294342041016f, 0.9333333373069763f, 1.0f);
            style.Colors[ImGuiCol_Tab] = ImVec4(0.1529411822557449f, 0.1725490242242813f, 0.2117647081613541f, 1.0f);
            style.Colors[ImGuiCol_TabHovered] = ImVec4(0.3098039329051971f, 0.6235294342041016f, 0.9333333373069763f, 0.250980406999588f);
            style.Colors[ImGuiCol_TabActive] = ImVec4(0.3098039329051971f, 0.6235294342041016f, 0.9333333373069763f, 1.0f);
            style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.1529411822557449f, 0.1725490242242813f, 0.2117647081613541f, 1.0f);
            style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.3098039329051971f, 0.6235294342041016f, 0.9333333373069763f, 1.0f);
            style.Colors[ImGuiCol_PlotLines] = ImVec4(0.6078431606292725f, 0.6078431606292725f, 0.6078431606292725f, 1.0f);
            style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.4274509847164154f, 0.3490196168422699f, 1.0f);
            style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8980392217636108f, 0.6980392336845398f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6000000238418579f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.105882354080677f, 0.1137254908680916f, 0.1372549086809158f, 1.0f);
            style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.2039215713739395f, 0.2313725501298904f, 0.2823529541492462f, 1.0f);
            style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2039215713739395f, 0.2313725501298904f, 0.2823529541492462f, 0.5021458864212036f);
            style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.03862661123275757f);
            style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.2039215713739395f, 0.2313725501298904f, 0.2823529541492462f, 1.0f);
            style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.8999999761581421f);
            style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
            style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.2039215713739395f, 0.2313725501298904f, 0.2823529541492462f, 0.7529411911964417f);
            style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.105882354080677f, 0.1137254908680916f, 0.1372549086809158f, 0.7529411911964417f);
            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.105882354080677f, 0.1137254908680916f, 0.1372549086809158f, 0.7529411911964417f);

            return style;
        }

        ImGuiStyle PhotoshopStyle()
        {
            // Photoshop style by Derydoca from ImThemes
            ImGuiStyle style;

            style.Alpha = 1.0f;
            style.DisabledAlpha = 0.6000000238418579f;
            style.WindowPadding = ImVec2(8.0f, 8.0f);
            style.WindowRounding = 4.0f;
            style.WindowBorderSize = 1.0f;
            style.WindowMinSize = ImVec2(32.0f, 32.0f);
            style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
            style.WindowMenuButtonPosition = ImGuiDir_Left;
            style.ChildRounding = 4.0f;
            style.ChildBorderSize = 1.0f;
            style.PopupRounding = 2.0f;
            style.PopupBorderSize = 1.0f;
            style.FramePadding = ImVec2(4.0f, 3.0f);
            style.FrameRounding = 2.0f;
            style.FrameBorderSize = 1.0f;
            style.ItemSpacing = ImVec2(8.0f, 4.0f);
            style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
            style.CellPadding = ImVec2(4.0f, 2.0f);
            style.IndentSpacing = 21.0f;
            style.ColumnsMinSpacing = 6.0f;
            style.ScrollbarSize = 13.0f;
            style.ScrollbarRounding = 12.0f;
            style.GrabMinSize = 7.0f;
            style.GrabRounding = 0.0f;
            style.TabRounding = 0.0f;
            style.TabBorderSize = 1.0f;
            style.TabMinWidthForCloseButton = 0.0f;
            style.ColorButtonPosition = ImGuiDir_Right;
            style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
            style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

            style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.4980392158031464f, 0.4980392158031464f, 0.4980392158031464f, 1.0f);
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1764705926179886f, 0.1764705926179886f, 0.1764705926179886f, 1.0f);
            style.Colors[ImGuiCol_ChildBg] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.2784313857555389f, 0.0f);
            style.Colors[ImGuiCol_PopupBg] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3098039329051971f, 1.0f);
            style.Colors[ImGuiCol_Border] = ImVec4(0.3627451121807098f, 0.3627451121807098f, 0.3627451121807098f, 0.6f);
            style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1568627506494522f * 1.5f, 0.1568627506494522f * 1.5f, 0.1568627506494522f * 1.5f, 1.0f);
            style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1568627506494522f * 1.6f, 0.1568627506494522f * 1.6f, 0.1568627506494522f * 1.6f, 1.0f);
            style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.2784313857555389f, 1.0f);
            style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1450980454683304f, 1.0f);
            style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1450980454683304f, 1.0f);
            style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1450980454683304f, 1.0f);
            style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.1921568661928177f, 0.1921568661928177f, 0.1921568661928177f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1568627506494522f, 0.1568627506494522f, 0.1568627506494522f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.2745098173618317f, 0.2745098173618317f, 0.2745098173618317f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.2980392277240753f, 0.2980392277240753f, 0.2980392277240753f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.3882353007793427f, 0.3882353007793427f, 0.3882353007793427f, 1.0f);
            style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 1.0f, 1.0f, 30.f / 255.f);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.1560000032186508f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.3910000026226044f);
            style.Colors[ImGuiCol_Header] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3098039329051971f, 1.0f);
            style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.4666666686534882f, 0.4666666686534882f, 0.4666666686534882f, 1.0f);
            style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.4666666686534882f, 0.4666666686534882f, 0.4666666686534882f, 1.0f);
            style.Colors[ImGuiCol_Separator] = ImVec4(0.2627451121807098f, 0.2627451121807098f, 0.2627451121807098f, 1.0f);
            style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.3882353007793427f, 0.3882353007793427f, 0.3882353007793427f, 1.0f);
            style.Colors[ImGuiCol_SeparatorActive] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.0f, 1.0f, 1.0f, 0.25f);
            style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.6700000166893005f);
            style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_Tab] = ImVec4(0.09411764889955521f, 0.09411764889955521f, 0.09411764889955521f, 1.0f);
            style.Colors[ImGuiCol_TabHovered] = ImVec4(0.3490196168422699f, 0.3490196168422699f, 0.3490196168422699f, 1.0f);
            style.Colors[ImGuiCol_TabActive] = ImVec4(0.1921568661928177f, 0.1921568661928177f, 0.1921568661928177f, 1.0f);
            style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.09411764889955521f, 0.09411764889955521f, 0.09411764889955521f, 1.0f);
            style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1921568661928177f, 0.1921568661928177f, 0.1921568661928177f, 1.0f);
            style.Colors[ImGuiCol_PlotLines] = ImVec4(0.4666666686534882f, 0.4666666686534882f, 0.4666666686534882f, 1.0f);
            style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.5843137502670288f, 0.5843137502670288f, 0.5843137502670288f, 1.0f);
            style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
            style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
            style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
            style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
            style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.0f, 1.0f, 1.0f, 0.1560000032186508f);
            style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_NavHighlight] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5860000252723694f);
            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5860000252723694f);

            style.FrameBorderSize = 1.f;
            style.FrameRounding = 1.f;

            return style;
        }

        ImGuiStyle ShadesOfGray(float rounding=0.f, float value_multiplier_front=1.f, float value_multiplier_bg=1.f)
        {
            ImGuiStyle style;

            style.Alpha = 1.0f;
            style.DisabledAlpha = 0.6000000238418579f;
            style.WindowRounding = 0.0f;

            style.Colors[ImGuiCol_Text] = ImVec4(0.09803921729326248f, 0.09803921729326248f, 0.09803921729326248f, 1.0f);
            style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.4980392158031464f, 0.4980392158031464f, 0.4980392158031464f, 1.0f);
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.9490196108818054f, 0.9490196108818054f, 0.9490196108818054f, 1.0f);
            style.Colors[ImGuiCol_ChildBg] = ImVec4(0.9490196108818054f, 0.9490196108818054f, 0.9490196108818054f, 1.0f);
            style.Colors[ImGuiCol_PopupBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_Border] = ImVec4(0.6000000238418579f, 0.6000000238418579f, 0.6000000238418579f, 1.0f);
            style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            style.Colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.0f, 0.4666666686534882f, 0.8392156958580017f, 0.2000000029802322f);
            style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.0f, 0.4666666686534882f, 0.8392156958580017f, 1.0f);
            style.Colors[ImGuiCol_TitleBg] = ImVec4(0.03921568766236305f, 0.03921568766236305f, 0.03921568766236305f, 1.0f);
            style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.1568627506494522f, 0.2862745225429535f, 0.47843137383461f, 1.0f);
            style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.5099999904632568f);
            style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.8588235378265381f, 0.8588235378265381f, 0.8588235378265381f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.8588235378265381f, 0.8588235378265381f, 0.8588235378265381f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.686274528503418f, 0.686274528503418f, 0.686274528503418f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.2000000029802322f);
            style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
            style.Colors[ImGuiCol_CheckMark] = ImVec4(0.09803921729326248f, 0.09803921729326248f, 0.09803921729326248f, 1.0f);
            style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.686274528503418f, 0.686274528503418f, 0.686274528503418f, 1.0f);
            style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
            style.Colors[ImGuiCol_Button] = ImVec4(0.8588235378265381f, 0.8588235378265381f, 0.8588235378265381f, 1.0f);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.0f, 0.4666666686534882f, 0.8392156958580017f, 0.2000000029802322f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.4666666686534882f, 0.8392156958580017f, 1.0f);
            style.Colors[ImGuiCol_Header] = ImVec4(0.8588235378265381f, 0.8588235378265381f, 0.8588235378265381f, 1.0f);
            style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.4666666686534882f, 0.8392156958580017f, 0.2000000029802322f);
            style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.4666666686534882f, 0.8392156958580017f, 1.0f);
            style.Colors[ImGuiCol_Separator] = ImVec4(0.4274509847164154f, 0.4274509847164154f, 0.4980392158031464f, 0.5f);
            style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.09803921729326248f, 0.4000000059604645f, 0.7490196228027344f, 0.7799999713897705f);
            style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.09803921729326248f, 0.4000000059604645f, 0.7490196228027344f, 1.0f);
            style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.2000000029802322f);
            style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.6700000166893005f);
            style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.949999988079071f);
            style.Colors[ImGuiCol_PlotLines] = ImVec4(0.6078431606292725f, 0.6078431606292725f, 0.6078431606292725f, 1.0f);
            style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.4274509847164154f, 0.3490196168422699f, 1.0f);
            style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8980392217636108f, 0.6980392336845398f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6000000238418579f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
            style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);

            style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f * 4.f, 0.1882352977991104f * 4.f, 0.2000000029802322f * 4.f, 1.0f);
            // style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.1882352977991104f * 3.f, 0.1882352977991104f * 3.f, 0.2000000029802322f * 3.f, 0.15f);

            style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
            style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.3499999940395355f);
            style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.8999999761581421f);
            style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
            style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
            style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);

            style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1764705926179886f, 0.3490196168422699f, 0.5764706134796143f, 0.8619999885559082f);
            style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.196078434586525f, 0.407843142747879f, 0.6784313917160034f, 1.0f);
            style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.06666667014360428f, 0.1019607856869698f, 0.1450980454683304f, 0.9724000096321106f);

            style.Colors[ImGuiCol_Tab]                    = ImVec4(0.78f, 0.79f, 0.81f, 0.86f);
            style.Colors[ImGuiCol_TabHovered]             = ImVec4(0.66f, 0.73f, 0.81f, 0.80f);
            style.Colors[ImGuiCol_TabActive]              = ImVec4(0.59f, 0.70f, 0.84f, 1.00f);
            style.Colors[ImGuiCol_TabUnfocused]           = ImVec4(0.64f, 0.74f, 0.86f, 0.97f);
            style.Colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.70f, 0.80f, 0.93f, 1.00f);

            ThemeTweakImpl::ApplySaturationMultiplier(style, 0.21f, style);

            ThemeTweakImpl::ApplyValueMultiplierBg(style, value_multiplier_bg, style);
            ThemeTweakImpl::ApplyValueMultiplierFront(style, value_multiplier_front, style);
            ThemeTweakImpl::ApplyRounding(style, rounding, 4.f);
            return style;
        }

        ImGuiStyle MicrosoftStyle()
        {
            ImGuiStyle style = ShadesOfGray();
            style.WindowPadding = ImVec2(4.0f, 6.0f);
            style.WindowBorderSize = 0.0f;
            style.WindowMinSize = ImVec2(32.0f, 32.0f);
            style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
            style.WindowMenuButtonPosition = ImGuiDir_Left;
            style.ChildRounding = 0.0f;
            style.ChildBorderSize = 1.0f;
            style.PopupRounding = 0.0f;
            style.PopupBorderSize = 1.0f;
            style.FramePadding = ImVec2(8.0f, 4.0f);
            style.FrameRounding = 0.0f;
            style.FrameBorderSize = 1.0f;
            style.ItemSpacing = ImVec2(8.0f, 6.0f);
            style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
            style.CellPadding = ImVec2(4.0f, 2.0f);
            style.IndentSpacing = 20.0f;
            style.ColumnsMinSpacing = 6.0f;
            style.ScrollbarSize = 20.0f;
            style.ScrollbarRounding = 0.0f;
            style.GrabMinSize = 5.0f;
            style.GrabRounding = 0.0f;
            style.TabRounding = 4.0f;
            style.TabBorderSize = 0.0f;
            style.TabMinWidthForCloseButton = 0.0f;
            style.ColorButtonPosition = ImGuiDir_Right;
            style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
            style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
            return style;
        }

        ImGuiStyle Cherry()
        {
            // Cherry style by r-lyeh from ImThemes
            ImGuiStyle style;

            style.Alpha = 1.0f;
            style.DisabledAlpha = 0.6000000238418579f;
            style.WindowPadding = ImVec2(6.0f, 3.0f);
            style.WindowRounding = 0.0f;
            style.WindowBorderSize = 1.0f;
            style.WindowMinSize = ImVec2(32.0f, 32.0f);
            style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
            style.WindowMenuButtonPosition = ImGuiDir_Left;
            style.ChildRounding = 0.0f;
            style.ChildBorderSize = 1.0f;
            style.PopupRounding = 0.0f;
            style.PopupBorderSize = 1.0f;
            style.FramePadding = ImVec2(5.0f, 1.0f);
            style.FrameRounding = 3.0f;
            style.FrameBorderSize = 1.0f;
            style.ItemSpacing = ImVec2(7.0f, 3.0f);
            style.ItemInnerSpacing = ImVec2(1.0f, 1.0f);
            style.CellPadding = ImVec2(4.0f, 2.0f);
            style.IndentSpacing = 6.0f;
            style.ColumnsMinSpacing = 6.0f;
            style.ScrollbarSize = 13.0f;
            style.ScrollbarRounding = 16.0f;
            style.GrabMinSize = 20.0f;
            style.GrabRounding = 2.0f;
            style.TabRounding = 4.0f;
            style.TabBorderSize = 1.0f;
            style.TabMinWidthForCloseButton = 0.0f;
            style.ColorButtonPosition = ImGuiDir_Right;
            style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
            style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

            style.Colors[ImGuiCol_Text] = ImVec4(0.8588235378265381f, 0.929411768913269f, 0.886274516582489f, 0.8799999952316284f);
            style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.8588235378265381f, 0.929411768913269f, 0.886274516582489f, 0.2800000011920929f);
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1294117718935013f, 0.1372549086809158f, 0.168627455830574f, 1.0f);
            style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            style.Colors[ImGuiCol_PopupBg] = ImVec4(0.2000000029802322f, 0.2196078449487686f, 0.2666666805744171f, 0.8999999761581421f);
            style.Colors[ImGuiCol_Border] = ImVec4(0.5372549295425415f, 0.47843137383461f, 0.2549019753932953f, 0.1620000004768372f);
            style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2000000029802322f, 0.2196078449487686f, 0.2666666805744171f, 1.0f);
            style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 0.7799999713897705f);
            style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
            style.Colors[ImGuiCol_TitleBg] = ImVec4(0.2313725501298904f, 0.2000000029802322f, 0.2705882489681244f, 1.0f);
            style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.501960813999176f, 0.07450980693101883f, 0.2549019753932953f, 1.0f);
            style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.2000000029802322f, 0.2196078449487686f, 0.2666666805744171f, 0.75f);
            style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.2000000029802322f, 0.2196078449487686f, 0.2666666805744171f, 0.4699999988079071f);
            style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.2000000029802322f, 0.2196078449487686f, 0.2666666805744171f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.08627451211214066f, 0.1490196138620377f, 0.1568627506494522f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 0.7799999713897705f);
            style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
            style.Colors[ImGuiCol_CheckMark] = ImVec4(0.7098039388656616f, 0.2196078449487686f, 0.2666666805744171f, 1.0f);
            style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.4666666686534882f, 0.7686274647712708f, 0.8274509906768799f, 0.1400000005960464f);
            style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.7098039388656616f, 0.2196078449487686f, 0.2666666805744171f, 1.0f);
            style.Colors[ImGuiCol_Button] = ImVec4(0.4666666686534882f, 0.7686274647712708f, 0.8274509906768799f, 0.1400000005960464f);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 0.8600000143051147f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
            style.Colors[ImGuiCol_Header] = ImVec4(0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 0.7599999904632568f);
            style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 0.8600000143051147f);
            style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.501960813999176f, 0.07450980693101883f, 0.2549019753932953f, 1.0f);
            style.Colors[ImGuiCol_Separator] = ImVec4(0.4274509847164154f, 0.4274509847164154f, 0.4980392158031464f, 0.5f);
            style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.09803921729326248f, 0.4000000059604645f, 0.7490196228027344f, 0.7799999713897705f);
            style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.09803921729326248f, 0.4000000059604645f, 0.7490196228027344f, 1.0f);
            style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.4666666686534882f, 0.7686274647712708f, 0.8274509906768799f, 0.03999999910593033f);
            style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 0.7799999713897705f);
            style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
            style.Colors[ImGuiCol_Tab] = ImVec4(0.1764705926179886f, 0.3490196168422699f, 0.5764706134796143f, 0.8619999885559082f);
            style.Colors[ImGuiCol_TabHovered] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.800000011920929f);
            style.Colors[ImGuiCol_TabActive] = ImVec4(0.196078434586525f, 0.407843142747879f, 0.6784313917160034f, 1.0f);
            style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.06666667014360428f, 0.1019607856869698f, 0.1450980454683304f, 0.9724000096321106f);
            style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1333333402872086f, 0.2588235437870026f, 0.4235294163227081f, 1.0f);
            style.Colors[ImGuiCol_PlotLines] = ImVec4(0.8588235378265381f, 0.929411768913269f, 0.886274516582489f, 0.6299999952316284f);
            style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
            style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8588235378265381f, 0.929411768913269f, 0.886274516582489f, 0.6299999952316284f);
            style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
            style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
            style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
            style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
            style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
            style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 0.4300000071525574f);
            style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.8999999761581421f);
            style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
            style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
            style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);

            ThemeTweakImpl::ApplyHue(style, 0.853f);
            ThemeTweakImpl::ApplyValueMultiplierFront(style, 0.971f, style);
            ThemeTweakImpl::ApplyValueMultiplierBg(style, 0.969f, style);
            ThemeTweakImpl::ApplyValueMultiplierText(style, 0.937f, style);
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1294117718935013f, 0.1372549086809158f, 0.168627455830574f, 1.0f);

            return style;
        }

        ImGuiStyle Darcula(
            float rounding=1.f,
            float hue=-1.f,
            float saturation_multiplier=1.f,
            float value_multiplier_front=1.f,
            float value_multiplier_bg=1.f,
            float alpha_bg_transparency=1.f
            )
        {
            // Darcula style by ice1000 from ImThemes
            ImGuiStyle style;

            style.Colors[ImGuiCol_Text] = ImVec4(0.7333333492279053f, 0.7333333492279053f, 0.7333333492279053f, 1.0f);
            style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.3450980484485626f, 0.3450980484485626f, 0.3450980484485626f, 1.0f);
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.2352941185235977f, 0.2470588237047195f, 0.2549019753932953f, 0.9399999976158142f);
            style.Colors[ImGuiCol_ChildBg] = ImVec4(0.2352941185235977f, 0.2470588237047195f, 0.2549019753932953f, 0.0f);
            style.Colors[ImGuiCol_PopupBg] = ImVec4(0.2352941185235977f, 0.2470588237047195f, 0.2549019753932953f, 0.9399999976158142f);
            style.Colors[ImGuiCol_Border] = ImVec4(0.5333333432674408f, 0.5333333432674408f, 0.5333333432674408f, 0.5f);
            style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.1568627506494522f, 0.1568627506494522f, 0.1568627506494522f, 0.0f);
            style.Colors[ImGuiCol_FrameBg] = ImVec4(0.168627455830574f, 0.168627455830574f, 0.168627455830574f, 0.5400000214576721f);
            style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.4509803950786591f, 0.6745098233222961f, 0.9960784316062927f, 0.6700000166893005f);
            style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.4705882370471954f, 0.4705882370471954f, 0.4705882370471954f, 0.6700000166893005f);
            style.Colors[ImGuiCol_TitleBg] = ImVec4(0.03921568766236305f, 0.03921568766236305f, 0.03921568766236305f, 1.0f);
            style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.5099999904632568f);
            style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1568627506494522f, 0.2862745225429535f, 0.47843137383461f, 1.0f);
            style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.2705882489681244f, 0.2862745225429535f, 0.2901960909366608f, 0.800000011920929f);
            style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.2705882489681244f, 0.2862745225429535f, 0.2901960909366608f, 0.6000000238418579f);
            style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.4196078449487686f, 0.4098039329051971f, 0.5196078479290009f, 0.5099999904632568f);
            style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.2196078449487686f, 0.3098039329051971f, 0.4196078479290009f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.1372549086809158f, 0.1921568661928177f, 0.2627451121807098f, 0.9100000262260437f);
            style.Colors[ImGuiCol_CheckMark] = ImVec4(0.8980392217636108f, 0.8980392217636108f, 0.8980392217636108f, 0.8299999833106995f);
            style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.6980392336845398f, 0.6980392336845398f, 0.6980392336845398f, 0.6200000047683716f);
            style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.2980392277240753f, 0.2980392277240753f, 0.2980392277240753f, 0.8399999737739563f);
            style.Colors[ImGuiCol_Button] = ImVec4(0.3333333432674408f, 0.3529411852359772f, 0.3607843220233917f, 0.4900000095367432f);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.2196078449487686f, 0.3098039329051971f, 0.4196078479290009f, 1.0f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1372549086809158f, 0.1921568661928177f, 0.2627451121807098f, 1.0f);
            style.Colors[ImGuiCol_Header] = ImVec4(0.3333333432674408f, 0.3529411852359772f, 0.3607843220233917f, 0.5299999713897705f);
            style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.4509803950786591f, 0.6745098233222961f, 0.9960784316062927f, 0.6700000166893005f);
            style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.4705882370471954f, 0.4705882370471954f, 0.4705882370471954f, 0.6700000166893005f);
            style.Colors[ImGuiCol_Separator] = ImVec4(0.3137255012989044f, 0.3137255012989044f, 0.3137255012989044f, 1.0f);
            style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.3137255012989044f, 0.3137255012989044f, 0.3137255012989044f, 1.0f);
            style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.3137255012989044f, 0.3137255012989044f, 0.3137255012989044f, 1.0f);
            style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.0f, 1.0f, 1.0f, 0.8500000238418579f);
            style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.6000000238418579f);
            style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.8999999761581421f);
            style.Colors[ImGuiCol_Tab] = ImVec4(0.1764705926179886f, 0.3490196168422699f, 0.5764706134796143f, 0.8619999885559082f);
            style.Colors[ImGuiCol_TabHovered] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.800000011920929f);
            style.Colors[ImGuiCol_TabActive] = ImVec4(0.196078434586525f, 0.407843142747879f, 0.6784313917160034f, 1.0f);
            style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.06666667014360428f, 0.1019607856869698f, 0.1450980454683304f, 0.9724000096321106f);
            style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1333333402872086f, 0.2588235437870026f, 0.4235294163227081f, 1.0f);
            style.Colors[ImGuiCol_PlotLines] = ImVec4(0.6078431606292725f, 0.6078431606292725f, 0.6078431606292725f, 1.0f);
            style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.4274509847164154f, 0.3490196168422699f, 1.0f);
            style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8980392217636108f, 0.6980392336845398f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6000000238418579f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
            style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
            style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
            style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
            style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.1843137294054031f, 0.3960784375667572f, 0.7921568751335144f, 0.8999999761581421f);
            style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.8999999761581421f);
            style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
            style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
            style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);
            style.Colors[ImGuiCol_ScrollbarGrabActive].z *= 1.5f;

            ThemeTweakImpl::ApplyRounding(style, rounding, 4.f);
            if (hue >= 0)
                ThemeTweakImpl::ApplyHue(style, hue);
            ThemeTweakImpl::ApplyValueMultiplierFront(style, value_multiplier_front, style);
            ThemeTweakImpl::ApplyValueMultiplierBg(style, value_multiplier_bg, style);
            ThemeTweakImpl::ApplySaturationMultiplier(style, saturation_multiplier, style);
            ThemeTweakImpl::ApplyAlphaMultiplier(style, alpha_bg_transparency, style);

            style.FrameBorderSize = 1.f;
            style.FrameRounding = 3.f;

                return style;
        }

        ImGuiStyle DarculaDarker()
        {
            float rounding = 3.f;
            float hue = 0.61f;
            float alpha_multiplier_bg_transparency = 0.92f;
            float saturation_multiplier = 0.993f;
            float value_multiplier_front = 0.981f;
            float value_multiplier_bg = 0.585f;

            ImGuiStyle style = Darcula(
                rounding,
                hue, saturation_multiplier,
                value_multiplier_front,
                value_multiplier_bg,
                alpha_multiplier_bg_transparency);
            ThemeTweakImpl::ApplyValueMultiplierFrameBg(style, 2.5f, style);
            style.Colors[ImGuiCol_Header] =
                ThemeTweakImpl::_ColorValueMultiply(style.Colors[ImGuiCol_Header], 1.4f);
            style.Colors[ImGuiCol_Text] = ImVec4(0.88f, 0.88f, 0.88f, 1.f);
            style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.6f, 0.6f, 0.6f, 1.f);
            style.Colors[ImGuiCol_FrameBg] = ImVec4(37.f /  255.f, 31.f /  255.f, 31.f /  255.f, 222.f /  255.f);
            return style;
        }

        ImGuiStyle LightRounded()
        {
            // Fork of Light style from ImThemes
            ImGuiStyle style;

            ThemeTweakImpl::ApplyRounding(style, 3.f, 3.f);

            style.Alpha = 1.0f;
            style.DisabledAlpha = 0.6000000238418579f;
            style.WindowPadding = ImVec2(9.100000381469727f, 8.0f);
            style.WindowBorderSize = 0.0f;
            style.WindowMinSize = ImVec2(20.0f, 32.0f);
            style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
            style.WindowMenuButtonPosition = ImGuiDir_Left;
            style.ChildBorderSize = 1.0f;
            style.PopupBorderSize = 0.0f;
            style.FramePadding = ImVec2(2.099999904632568f, 1.0f);
            style.FrameBorderSize = 0.0f;
            style.ItemSpacing = ImVec2(3.299999952316284f, 3.700000047683716f);
            style.ItemInnerSpacing = ImVec2(3.200000047683716f, 4.0f);
            style.CellPadding = ImVec2(2.799999952316284f, 1.0f);
            style.IndentSpacing = 9.199999809265137f;
            style.ColumnsMinSpacing = 6.300000190734863f;
            style.ScrollbarSize = 15.5f;
            style.GrabMinSize = 10.89999961853027f;
            style.TabBorderSize = 1.0f;
            style.TabMinWidthForCloseButton = 0.0f;
            style.ColorButtonPosition = ImGuiDir_Right;
            style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
            style.SelectableTextAlign = ImVec2(0.0f, 0.5f);

            style.Colors[ImGuiCol_Text] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.6000000238418579f, 0.6000000238418579f, 0.6000000238418579f, 1.0f);
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.9613733887672424f, 0.9531213045120239f, 0.9531213045120239f, 1.0f);
            style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            style.Colors[ImGuiCol_PopupBg] = ImVec4(1.0f, 1.0f, 1.0f, 0.9800000190734863f);
            style.Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 0.300000011920929f);
            style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            style.Colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.4000000059604645f);
            style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.6700000166893005f);
            style.Colors[ImGuiCol_TitleBg] = ImVec4(0.95686274766922f, 0.95686274766922f, 0.95686274766922f, 1.0f);
            style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.8196078538894653f, 0.8196078538894653f, 0.8196078538894653f, 1.0f);
            style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 1.0f, 1.0f, 0.5099999904632568f);
            style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.8588235378265381f, 0.8588235378265381f, 0.8588235378265381f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.9764705896377563f, 0.9764705896377563f, 0.9764705896377563f, 0.5299999713897705f);
            style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.686274528503418f, 0.686274528503418f, 0.686274528503418f, 0.800000011920929f);
            style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.4862745106220245f, 0.4862745106220245f, 0.4862745106220245f, 0.800000011920929f);
            style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.4862745106220245f, 0.4862745106220245f, 0.4862745106220245f, 1.0f);
            style.Colors[ImGuiCol_CheckMark] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
            style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.7799999713897705f);
            style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.4588235318660736f, 0.5372549295425415f, 0.800000011920929f, 0.6000000238418579f);
            style.Colors[ImGuiCol_Button] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.4000000059604645f);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.05882352963089943f, 0.529411792755127f, 0.9764705896377563f, 1.0f);
            style.Colors[ImGuiCol_Header] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.3100000023841858f);
            style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.800000011920929f);
            style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
            style.Colors[ImGuiCol_Separator] = ImVec4(0.3882353007793427f, 0.3882353007793427f, 0.3882353007793427f, 0.6200000047683716f);
            style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.1372549086809158f, 0.4392156898975372f, 0.800000011920929f, 0.7799999713897705f);
            style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.1372549086809158f, 0.4392156898975372f, 0.800000011920929f, 1.0f);
            style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.3490196168422699f, 0.3490196168422699f, 0.3490196168422699f, 0.1700000017881393f);
            style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.6700000166893005f);
            style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.949999988079071f);
            style.Colors[ImGuiCol_Tab] = ImVec4(0.7607843279838562f, 0.7960784435272217f, 0.8352941274642944f, 0.9309999942779541f);
            style.Colors[ImGuiCol_TabHovered] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.800000011920929f);
            style.Colors[ImGuiCol_TabActive] = ImVec4(0.5921568870544434f, 0.7254902124404907f, 0.8823529481887817f, 1.0f);
            style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.9176470637321472f, 0.9254902005195618f, 0.9333333373069763f, 0.9861999750137329f);
            style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.7411764860153198f, 0.8196078538894653f, 0.9137254953384399f, 1.0f);
            style.Colors[ImGuiCol_PlotLines] = ImVec4(0.3882353007793427f, 0.3882353007793427f, 0.3882353007793427f, 1.0f);
            style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.4274509847164154f, 0.3490196168422699f, 1.0f);
            style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8980392217636108f, 0.6980392336845398f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.4470588266849518f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.7764706015586853f, 0.8666666746139526f, 0.9764705896377563f, 1.0f);
            style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.5686274766921997f, 0.5686274766921997f, 0.6392157077789307f, 1.0f);
            style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.6784313917160034f, 0.6784313917160034f, 0.7372549176216125f, 1.0f);
            style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.2980392277240753f, 0.2980392277240753f, 0.2980392277240753f, 0.09000000357627869f);
            style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.3499999940395355f);
            style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.949999988079071f);
            style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.800000011920929f);
            style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.6980392336845398f, 0.6980392336845398f, 0.6980392336845398f, 0.699999988079071f);
            style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2000000029802322f, 0.2000000029802322f);
            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2000000029802322f, 0.3499999940395355f);

            return style;
        }

        ImGuiStyle WhiteIsWhite()
        {
            ImGuiStyle style = ShadesOfGray();
            ThemeTweakImpl::ApplyValueMultiplierFront(style, 0.94f, style);
            ThemeTweakImpl::ApplyValueMultiplierBg(style, 7.f, style);
            ThemeTweakImpl::ApplyValueMultiplierFrameBg(style, 0.91f, style);
            style.Colors[ImGuiCol_FrameBg] = ThemeTweakImpl::_ColorSetValue(style.Colors[ImGuiCol_FrameBg], 0.99f);
            style.FrameBorderSize = 1.f;
            style.FrameRounding = 3.f;
            return style;
        }

        ImGuiStyle BlackIsBlack()
        {
            ImGuiStyle style = SoDark(0.548f);
            ThemeTweakImpl::ApplyValueMultiplierFront(style, 1.356f, style);
            ThemeTweakImpl::ApplyValueMultiplierBg(style, 0.f, style);
            ThemeTweakImpl::ApplyValueMultiplierFrameBg(style, 1.12f, style);
            style.Colors[ImGuiCol_FrameBg] = ImVec4(27.f / 255.f, 27.f / 255.f, 27.f / 255.f, 231.f / 255.f);
            style.Colors[ImGuiCol_Header] = ImVec4(0.25f, 0.25f, 0.25f, 1.f);
            return style;
        }
    } // namespace ThemesImpl

    struct ThemeInfo
    {
        ImGuiTheme_ Theme;
        char Name[256];
        ImGuiStyle Style;
    };

    ThemeInfo gThemeInfos[] = {
        {ImGuiTheme_ImGuiColorsClassic,    "ImGuiColorsClassic",  ThemesImpl::ImGui_StyleColorsClassic() },
        {ImGuiTheme_ImGuiColorsLight,      "ImGuiColorsLight",    ThemesImpl::ImGui_StyleColorsLight() },
        {ImGuiTheme_ImGuiColorsDark,       "ImGuiColorsDark",     ThemesImpl::ImGui_StyleColorsDark()},
        {ImGuiTheme_MaterialFlat,          "MaterialFlat",        ThemesImpl::MaterialFlat()},
        {ImGuiTheme_PhotoshopStyle,        "PhotoshopStyle",      ThemesImpl::PhotoshopStyle()},
        {ImGuiTheme_GrayVariations,        "GrayVariations",        ThemesImpl::ShadesOfGray(3.f, 1.f, 1.f)},
        {ImGuiTheme_GrayVariations_Darker, "GrayVariations_Darker",  ThemesImpl::ShadesOfGray(3.f, 1.136f, 0.865f)},
        {ImGuiTheme_MicrosoftStyle,        "MicrosoftStyle",      ThemesImpl::MicrosoftStyle()},
        {ImGuiTheme_Cherry,                "Cherry",              ThemesImpl::Cherry()},
        {ImGuiTheme_Darcula,               "Darcula",             ThemesImpl::Darcula()},
        {ImGuiTheme_DarculaDarker,         "DarculaDarker",       ThemesImpl::DarculaDarker()},
        {ImGuiTheme_LightRounded,          "LightRounded",        ThemesImpl::LightRounded()},
        {ImGuiTheme_SoDark_AccentBlue,     "SoDark_AccentBlue",   ThemesImpl::SoDark(0.548f)},
        {ImGuiTheme_SoDark_AccentYellow,   "SoDark_AccentYellow", ThemesImpl::SoDark(0.163f)},
        {ImGuiTheme_SoDark_AccentRed,      "SoDark_AccentRed",    ThemesImpl::SoDark(0.f)},
        {ImGuiTheme_BlackIsBlack,          "BlackIsBlack",        ThemesImpl::BlackIsBlack()},
        {ImGuiTheme_WhiteIsWhite,          "WhiteIsWhite",        ThemesImpl::WhiteIsWhite()},
    };

    const char* ImGuiTheme_Name(ImGuiTheme_ theme)
    {
        IM_ASSERT((theme >= 0) && (theme < ImGuiTheme_Count));
        for (size_t i = 0; i < IM_ARRAYSIZE(gThemeInfos); ++i)
        {
            if (gThemeInfos[i].Theme == theme)
                return gThemeInfos[i].Name;
        }
        return "";
    }
    ImGuiTheme_ ImGuiTheme_FromName(const char* themeName)
    {
        std::string themeStr = themeName;
        for (size_t i = 0; i < IM_ARRAYSIZE(gThemeInfos); ++i)
        {
            if (gThemeInfos[i].Name == themeStr)
                return gThemeInfos[i].Theme;
        }
        return ImGuiTheme_ImGuiColorsClassic;
    }


    ImGuiStyle ThemeToStyle(ImGuiTheme_ theme)
    {
        IM_ASSERT((theme >= 0) && (theme < ImGuiTheme_Count));
        for (size_t i = 0; i < IM_ARRAYSIZE(gThemeInfos); ++i)
        {
            if (gThemeInfos[i].Theme == theme)
                return gThemeInfos[i].Style;
        }
        return ImGuiStyle();
    }

    void ApplyTheme(ImGuiTheme_ theme)
    {
        ImGuiStyle style = ThemeToStyle(theme);
        ImGui::GetStyle() = style;
    }

    ImGuiStyle TweakedThemeThemeToStyle(const ImGuiTweakedTheme& tweaked_theme)
    {
        ImGuiStyle style = ThemeToStyle(tweaked_theme.Theme);
        ImGuiStyle reference_style = style;

        const ImGuiThemeTweaks& tweaks = tweaked_theme.Tweaks;

        if (tweaks.Rounding >= 0.f)
            ThemeTweakImpl::ApplyRounding(style, tweaks.Rounding, tweaks.RoundingScrollbarRatio);
        if (tweaks.AlphaMultiplier >= 0.f)
            ThemeTweakImpl::ApplyAlphaMultiplier(style, tweaks.AlphaMultiplier, reference_style);
        if (tweaks.Hue >= 0.f)
            ThemeTweakImpl::ApplyHue(style, tweaks.Hue);
        if (tweaks.SaturationMultiplier >= 0.f)
            ThemeTweakImpl::ApplySaturationMultiplier(style, tweaks.SaturationMultiplier, reference_style);
        if (tweaks.ValueMultiplierFront >= 0.f)
            ThemeTweakImpl::ApplyValueMultiplierFront(style, tweaks.ValueMultiplierFront, reference_style);
        if (tweaks.ValueMultiplierBg >= 0.f)
            ThemeTweakImpl::ApplyValueMultiplierBg(style, tweaks.ValueMultiplierBg, reference_style);
        if (tweaks.ValueMultiplierText >= 0.f)
            ThemeTweakImpl::ApplyValueMultiplierText(style, tweaks.ValueMultiplierText, reference_style);
        if (tweaks.ValueMultiplierFrameBg >= 0.f)
            ThemeTweakImpl::ApplyValueMultiplierFrameBg(style, tweaks.ValueMultiplierFrameBg, reference_style);

        return style;
    }

    void ApplyTweakedTheme(const ImGuiTweakedTheme& tweaked_theme)
    {
        ImGui::GetStyle() = TweakedThemeThemeToStyle(tweaked_theme);
    }

    std::stack<ImGuiStyle> gPreviousStyles_PushTweakedTheme;

    void PushTweakedTheme(const ImGuiTweakedTheme& tweaked_theme)
    {
        gPreviousStyles_PushTweakedTheme.push(ImGui::GetStyle());
        ApplyTweakedTheme(tweaked_theme);

        bool tooManyThemes = gPreviousStyles_PushTweakedTheme.size() > 10;
        if (tooManyThemes)
            IM_ASSERT(false && "Too many PushTweakedTheme() calls without matching PopTweakedTheme()");
    }

    void PopTweakedTheme()
    {
        IM_ASSERT(!gPreviousStyles_PushTweakedTheme.empty());
        ImGui::GetStyle() = gPreviousStyles_PushTweakedTheme.top();
        gPreviousStyles_PushTweakedTheme.pop();
    }


    bool _ShowThemeSelector(ImGuiTheme_* theme)
    {
        bool changed = false;
        ImVec2 listboxSize =  ImVec2(15.f * ImGui::GetFontSize(), ImGuiTheme_Count * (ImGui::GetFontSize() + ImGui::GetStyle().ItemInnerSpacing.y) );
        ImGui::Text("Available Themes");
        if (ImGui::BeginListBox("##Available_themes", listboxSize))
        {
            int nbThemes = ImGuiTheme_Count;
            // We start at 3 because we want to place the older themes at the end
            for (int i = 3; i < nbThemes + 3; ++i)
            {
                ImGuiTheme_ theme_i = (ImGuiTheme_)(i % nbThemes);
                const bool is_selected = (*theme == theme_i);
                if (ImGui::Selectable(ImGuiTheme_Name(theme_i), is_selected))
                {
                    changed = true;
                    *theme = theme_i;
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndListBox();
        }
        return changed;
    }

    bool _ShowTweakGui(ImGuiThemeTweaks* tweaks)
    {
        ImGui::SeparatorText("Main tweaks");
        bool changed = false;
        {
            bool active = tweaks->AlphaMultiplier >= 0;
            if (ImGui::Checkbox("Alpha Multiplier / Background transparency", &active))
                changed = true;

            if (active)
            {
                if (tweaks->AlphaMultiplier < 0.f)
                    tweaks->AlphaMultiplier = 1.f;
                if (ImGui::SliderFloat("Alpha multiplier", &tweaks->AlphaMultiplier, 0.f, 2.f))
                    changed = true;
            }
            else
                tweaks->AlphaMultiplier = -1.f;
        }

        {
            bool active = tweaks->Rounding >= 0;
            if (ImGui::Checkbox("Rounding / all widgets", &active))
                changed = true;

            if (active)
            {
                if (tweaks->Rounding < 0.f)
                    tweaks->Rounding = 0.f;
                if (ImGui::SliderFloat("Rounding", &tweaks->Rounding, 0.f, 20.f))
                    changed = true;
                if (ImGui::SliderFloat("Rounding scrollbar ratio", &tweaks->RoundingScrollbarRatio, 0.f, 20.f))
                    changed = true;
            }
            else
                tweaks->Rounding = -1.f;
        }

        ImGui::SeparatorText("Color theme");
        {
            // Hue
            {
                bool active = tweaks->Hue >= 0;
                if (ImGui::Checkbox("Hue / all widgets", &active))
                    changed = true;

                if (active)
                {
                    if (tweaks->Hue < 0.f)
                        tweaks->Hue = 0.f;
                    if (ImGui::SliderFloat("Hue", &tweaks->Hue, 0.f, 1.f))
                        changed = true;
                }
                else
                    tweaks->Hue = -1.f;
            }
            // Saturation multiplier
            {
                bool active = tweaks->SaturationMultiplier >= 0;
                if (ImGui::Checkbox("Saturation multiplier / all widgets", &active))
                    changed = true;

                if (active)
                {
                    if (tweaks->SaturationMultiplier < 0.f)
                        tweaks->SaturationMultiplier = 1.f;
                    if (ImGui::SliderFloat("Saturation multiplier", &tweaks->SaturationMultiplier, 0.f, 3.f))
                        changed = true;
                }
                else
                    tweaks->SaturationMultiplier = -1.f;
            }
            // Value multiplier
            {
                bool active = tweaks->ValueMultiplierFront >= 0;
                if (ImGui::Checkbox("Value multiplier / widgets", &active))
                    changed = true;

                if (active)
                {
                    if (tweaks->ValueMultiplierFront < 0.f)
                        tweaks->ValueMultiplierFront = 1.f;
                    if (ImGui::SliderFloat("Value multiplier Front", &tweaks->ValueMultiplierFront, 0.f, 3.f))
                        changed = true;
                }
                else
                    tweaks->ValueMultiplierFront = -1.f;
            }
            // ValueMultiplierBg
            {
                bool active = tweaks->ValueMultiplierBg >= 0;
                if (ImGui::Checkbox("Value multiplier / backgrounds", &active))
                    changed = true;

                if (active)
                {
                    if (tweaks->ValueMultiplierBg < 0.f)
                        tweaks->ValueMultiplierBg = 1.f;
                    if (ImGui::SliderFloat("Value multiplier Bg", &tweaks->ValueMultiplierBg, 0.f, 10.f))
                        changed = true;
                }
                else
                    tweaks->ValueMultiplierBg = -1.f;
            }
            // ValueMultiplierText
            {
                bool active = tweaks->ValueMultiplierText >= 0;
                if (ImGui::Checkbox("Value multiplier / texts", &active))
                    changed = true;

                if (active)
                {
                    if (tweaks->ValueMultiplierText < 0.f)
                        tweaks->ValueMultiplierText = 1.f;
                    if (ImGui::SliderFloat("Value multiplier texts", &tweaks->ValueMultiplierText, 0.f, 10.f))
                        changed = true;
                }
                else
                    tweaks->ValueMultiplierText = -1.f;
            }
            // ValueMultiplierFrameBg
            {
                bool active = tweaks->ValueMultiplierFrameBg >= 0;
                if (ImGui::Checkbox("Value multiplier / FrameBg", &active))
                    changed = true;

                if (active)
                {
                    if (tweaks->ValueMultiplierFrameBg < 0.f)
                        tweaks->ValueMultiplierFrameBg = 1.f;
                    if (ImGui::SliderFloat("Value multiplier FrameBg", &tweaks->ValueMultiplierFrameBg, 0.f, 10.f))
                        changed = true;
                }
                else
                    tweaks->ValueMultiplierFrameBg = -1.f;
            }

        }
        return changed;
    }

    bool ShowThemeTweakGui(ImGuiTweakedTheme *tweaked_theme)
    {
        bool changed = false;

        static bool showDemoWindow = false;
        ImGui::Checkbox("Test on ImGui Demo Window", &showDemoWindow);
        if (showDemoWindow)
            ImGui::ShowDemoWindow(&showDemoWindow);

        if (ImGui::BeginTabBar("Blah"))
        {
            if (ImGui::BeginTabItem("Theme Tweaks"))
            {
                if (_ShowThemeSelector(&tweaked_theme->Theme))
                    changed = true;
                if (_ShowTweakGui(&tweaked_theme->Tweaks))
                    changed = true;
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Style Editor"))
            {
                ImGui::ShowStyleEditor(nullptr);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        return changed;
    }

    ImGuiStyle SoDark(float hue)
    {
        return ThemesImpl::SoDark(hue);
    }

    ImGuiStyle ShadesOfGray(float rounding, float value_multiplier_front, float value_multiplier_bg)
    {
        return ThemesImpl::ShadesOfGray(rounding, value_multiplier_front, value_multiplier_bg);
    }

    ImGuiStyle Darcula(
        float rounding,
        float hue,
        float saturation_multiplier,
        float value_multiplier_front,
        float value_multiplier_bg,
        float alpha_bg_transparency
    )
    {
        return ThemesImpl::Darcula(rounding, hue, saturation_multiplier, value_multiplier_front, value_multiplier_bg, alpha_bg_transparency);
    }

} // namespace ImGuiTheme
