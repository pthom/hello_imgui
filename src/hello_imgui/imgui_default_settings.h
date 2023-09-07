#pragma once
#include "hello_imgui/runner_callbacks.h"
#include "imgui.h"
#include <string>

namespace HelloImGui
{
ImFont* LoadFontTTF(const std::string & fontFilename, float fontSize, bool useFullGlyphRange = false, ImFontConfig config = ImFontConfig());
ImFont* LoadFontTTF_WithFontAwesomeIcons(const std::string & fontFilename, float fontSize, bool useFullGlyphRange = false, ImFontConfig configFont = ImFontConfig(), ImFontConfig configIcons = ImFontConfig());
ImFont* MergeFontAwesomeToLastFont(float fontSize, ImFontConfig config = ImFontConfig());

namespace ImGuiDefaultSettings
{
// LoadDefaultFont_WithFontAwesome will load from assets/fonts and reverts to the imgui embedded font if not found.
void LoadDefaultFont_WithFontAwesomeIcons();

void SetupDefaultImGuiConfig();
void SetupDefaultImGuiStyle();

// indicates that fonts were loaded using HelloImGui::LoadFontTTF. In that case, fonts may have been resized to
// account for HighDPI (on macOS and emscripten)
bool DidCallHelloImGuiLoadFontTTF();
}  // namespace ImGuiDefaultSettings
}  // namespace HelloImGui
