#pragma once
#include "hello_imgui/imgui_default_settings.h"
#include <functional>

namespace HelloImGui
{
/// GuiFunctionPointer ; any function that displays a GUI via ImGui
///and returns false most of the time, except if exit is required
/// (following a user action for example)
using GuiFunctionPointer = std::function<void(void)>;
/// VoidFunctionPointer: any void(void) function
using VoidFunctionPointer = std::function<void(void)>;
/// NoAction: empty void(void) function (aka VoidFunctionPointer)
inline void NoAction() {}


struct RunnerCallbacks
{
    /// Fill ShowGui with a lambda or function that will render your own gui!
    GuiFunctionPointer ShowGui = []() {
            // This implementation should be overriden in your code
            // by setting showGui
            ImGui::Text("Hello, World");
            return false;
    };

    /// Fill ShowMenus() with a function that will add ImGui menus
    /// by calling:
    ///    ImGui::BeginMenu(...) / ImGui::MenuItem(...) / ImGui::EndMenu()
    GuiFunctionPointer ShowMenus = {};

    /// If needed, fill PostInit with a function that will be called once
    /// after OpenGl and ImGui are inited
    VoidFunctionPointer PostInit = NoAction;
    /// If needed, fill LoadAdditionalFonts with your own
    /// font loading code
    VoidFunctionPointer LoadAdditionalFonts = ImGuiDefaultSettings::LoadDefaultFont_WithFontAwesome;
    /// If needed, change ImGui config via SetupImGuiConfig
    /// (enable docking, gamepad, etc)
    VoidFunctionPointer SetupImGuiConfig = ImGuiDefaultSettings::SetupDefaultImGuiConfig;
    /// If needed, setup your own style by providing your own SetupImGuiStyle callback
    VoidFunctionPointer SetupImGuiStyle = ImGuiDefaultSettings::SetupDefaultImGuiStyle;
};

}  // namespace HelloImGui
