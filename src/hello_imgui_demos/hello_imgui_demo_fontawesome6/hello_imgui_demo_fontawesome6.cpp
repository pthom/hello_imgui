// Prevent HelloImGui from loading Font Awesome 4 definitions, since we will use FontAwesome 6
#define HELLOIMGUI_NO_FONT_AWESOME4
#include "hello_imgui/hello_imgui.h"

// Found at https://raw.githubusercontent.com/juliettef/IconFontCppHeaders/main/IconsFontAwesome6.h
#include "IconsFontAwesome6.h"

#include "imgui.h"


int main(int, char**)
{
    HelloImGui::RunnerParams runnerParams;

    runnerParams.callbacks.LoadAdditionalFonts = []
    {
        // Load the default font
        HelloImGui::LoadFont("fonts/DroidSans.ttf", 15.0f);

        // Merge FontAwesome6 with the default font
        HelloImGui::FontLoadingParams fontParams;
        fontParams.mergeToLastFont = true;
        fontParams.useFullGlyphRange = true;
        HelloImGui::LoadFont("fonts/Font_Awesome_6_Free-Solid-900.otf", 15.0f, fontParams);
    };

    auto gui = []() {

        ImGui::Text("Hello, world!" );
        ImGui::Text(ICON_FA_FACE_SMILE);
        ImGui::Text(ICON_FA_LAYER_GROUP);
        ImGui::Text(ICON_FA_ADDRESS_CARD);
    };

    runnerParams.callbacks.ShowGui = gui;

    HelloImGui::Run(runnerParams);
}