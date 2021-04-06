#pragma once
#include <string>
namespace HelloImGui
{
ImFont* LoadFontTTF(const std::string & fontFilename, float fontSize, bool useFullGlyphRange = false, ImFontConfig config = ImFontConfig());
ImFont* LoadFontTTF_WithFontAwesomeIcons(const std::string & fontFilename, float fontSize, bool useFullGlyphRange = false, ImFontConfig configFont = ImFontConfig(), ImFontConfig configIcons = ImFontConfig());
ImFont* MergeFontAwesomeToLastFont(float fontSize, ImFontConfig config = ImFontConfig());

namespace ImGuiDefaultSettings
{
void LoadDefaultFont_WithFontAwesomeIcons();
void SetupDefaultImGuiConfig();
void SetupDefaultImGuiStyle();
}  // namespace ImGuiDefaultSettings
}  // namespace HelloImGui
