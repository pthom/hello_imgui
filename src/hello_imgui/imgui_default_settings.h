#pragma once


namespace HelloImGui
{

    namespace ImGuiDefaultSettings
    {
        // LoadDefaultFont_WithFontAwesome will load from assets/fonts and reverts to the imgui embedded font if not found.
        void LoadDefaultFont_WithFontAwesomeIcons();

        void SetupDefaultImGuiConfig();
        void SetupDefaultImGuiStyle();
    }  // namespace ImGuiDefaultSettings

}  // namespace HelloImGui
