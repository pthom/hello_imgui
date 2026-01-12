#pragma once
#include "imgui.h"

//
// Theme tweak utilities for ImGui.
// Reuse and adaptation of imgui_theme.h and imgui_theme.cpp file is granted for other projects,
// provided the origin of those files is stated in the copied version
// Some themes were adapted by themes posted by ImGui users at https://github.com/ocornut/imgui/issues/707
//

namespace ImGuiTheme
{
    enum ImGuiTheme_
    {
        ImGuiTheme_ImGuiColorsClassic = 0,
        ImGuiTheme_ImGuiColorsDark,
        ImGuiTheme_ImGuiColorsLight,
        ImGuiTheme_MaterialFlat,
        ImGuiTheme_PhotoshopStyle,
        ImGuiTheme_GrayVariations,
        ImGuiTheme_GrayVariations_Darker,
        ImGuiTheme_MicrosoftStyle,
        ImGuiTheme_Cherry,
        ImGuiTheme_Darcula,
        ImGuiTheme_DarculaDarker,
        ImGuiTheme_LightRounded,
        ImGuiTheme_SoDark_AccentBlue,
        ImGuiTheme_SoDark_AccentYellow,
        ImGuiTheme_SoDark_AccentRed,
        ImGuiTheme_BlackIsBlack,
        ImGuiTheme_WhiteIsWhite,
        ImGuiTheme_Count
    };
    const char* ImGuiTheme_Name(ImGuiTheme_ theme);
    ImGuiTheme_ ImGuiTheme_FromName(const char* themeName);
    ImGuiStyle ThemeToStyle(ImGuiTheme_ theme);
    void ApplyTheme(ImGuiTheme_ theme);


    struct ImGuiThemeTweaks
    {
        // Common rounding for widgets. If < 0, this is ignored.
        float Rounding = -1.f;
        // If rounding is applied, scrollbar rounding needs to be adjusted to be visually pleasing in conjunction with other widgets roundings. Only applied if Rounding > 0.f)
        float RoundingScrollbarRatio = 4.f;
        // Change the alpha that will be applied to windows, popups, etc. If < 0, this is ignored.
        float AlphaMultiplier = -1.f;

        //
        // HSV Color tweaks
        //
        // Change the hue of all widgets (gray widgets will remain gray, since their saturation is zero). If < 0, this is ignored.
        float Hue = -1.f;
        // Multiply the saturation of all widgets (gray widgets will remain gray, since their saturation is zero). If < 0, this is ignored.
        float SaturationMultiplier = -1.f;
        // Multiply the value (luminance) of all front widgets. If < 0, this is ignored.
        float ValueMultiplierFront = -1.f;
        // Multiply the value (luminance) of all backgrounds. If < 0, this is ignored.
        float ValueMultiplierBg = -1.f;
        // Multiply the value (luminance) of text. If < 0, this is ignored.
        float ValueMultiplierText = -1.f;
        // Multiply the value (luminance) of FrameBg. If < 0, this is ignored.
        // (Background of checkbox, radio button, plot, slider, text input)
        float ValueMultiplierFrameBg = -1.f;

        ImGuiThemeTweaks() {}
    };

    struct ImGuiTweakedTheme
    {
        ImGuiTheme_ Theme = ImGuiTheme_DarculaDarker;
        ImGuiThemeTweaks Tweaks = ImGuiThemeTweaks();

        ImGuiTweakedTheme(ImGuiTheme_ theme = ImGuiTheme_DarculaDarker, const ImGuiThemeTweaks& tweaks = ImGuiThemeTweaks())
            : Theme(theme), Tweaks(tweaks) {}
    };

    ImGuiStyle TweakedThemeThemeToStyle(const ImGuiTweakedTheme& tweaked_theme);
    void ApplyTweakedTheme(const ImGuiTweakedTheme& tweaked_theme);

    // PushTweakedTheme() / PopTweakedTheme()
    // Push and pop a tweaked theme
    //
    // Note: If you want the theme to apply globally to a window, you need to apply it
    //       *before* calling ImGui::Begin
    //
    //     For example, within Hello ImGui, given a dockable window, you should set this option:
    //        myDockableWindow.callBeginEnd = false;
    //     And then:
    //     - call ImGuiTheme::PushTweakedTheme
    //     - call ImGui::Begin
    //     - display your content
    //     - call ImGui::End
    //     - call ImGuiTheme::PopTweakedTheme
    //
    // See demo inside src/hello_imgui_demos/hello_imgui_demodocking/hello_imgui_demodocking.main.cpp:
    //     look at `GuiWindowAlternativeTheme()`
    void PushTweakedTheme(const ImGuiTweakedTheme& tweaked_theme);
    void PopTweakedTheme();

    // Show the theme selection listbox, the theme tweak widgets, as well as ImGui::ShowStyleEditor. Returns true if modified (Warning, when using ShowStyleEditor, no info about modification is transmitted)
    bool ShowThemeTweakGui(ImGuiTweakedTheme *tweaked_theme);

    // Some tweakable themes
    ImGuiStyle SoDark(float hue);
    ImGuiStyle ShadesOfGray(float rounding=0.f, float value_multiplier_front=1.f, float value_multiplier_bg=1.f);
    ImGuiStyle Darcula(
        float rounding=1.f,
        float hue=-1.f,
        float saturation_multiplier=1.f,
        float value_multiplier_front=1.f,
        float value_multiplier_bg=1.f,
        float alpha_bg_transparency=1.f
    );


} // namespace ImGuiTheme