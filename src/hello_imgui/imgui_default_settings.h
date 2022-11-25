#pragma once
#include "hello_imgui/runner_callbacks.h"
#include <string>

namespace HelloImGui
{
ImFont* LoadFontTTF(const std::string & fontFilename, float fontSize, bool useFullGlyphRange = false, ImFontConfig config = ImFontConfig());
ImFont* LoadFontTTF_WithFontAwesomeIcons(const std::string & fontFilename, float fontSize, bool useFullGlyphRange = false, ImFontConfig configFont = ImFontConfig(), ImFontConfig configIcons = ImFontConfig());
ImFont* MergeFontAwesomeToLastFont(float fontSize, ImFontConfig config = ImFontConfig());

namespace ImGuiDefaultSettings
{
VoidFunction LoadDefaultFont_WithFontAwesomeIcons();
VoidFunction SetupDefaultImGuiConfig();
VoidFunction SetupDefaultImGuiStyle();
}  // namespace ImGuiDefaultSettings
}  // namespace HelloImGui
