#pragma once
#include <string>
namespace HelloImGui
{
ImFont* LoadFontTTF(const std::string & fontFilename, float fontSize);
ImFont* LoadFontTTF_WithFontAwesomeIcons(const std::string & fontFilename, float fontSize);

namespace ImGuiDefaultSettings
{
void LoadDefaultFont_WithFontAwesomeIcons();
void SetupDefaultImGuiConfig();
void SetupDefaultImGuiStyle();
}  // namespace ImGuiDefaultSettings
}  // namespace HelloImGui
