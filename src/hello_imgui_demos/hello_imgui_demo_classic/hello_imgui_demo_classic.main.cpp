#include "hello_imgui/hello_imgui.h"


// Demonstrate how to load additional fonts (fonts - part 1/3)
HelloImGui::FontDpiResponsive * gCustomFont = nullptr;
void MyLoadFonts()
{
	HelloImGui::GetRunnerParams()->dpiAwareParams.onlyUseFontDpiResponsive = true;
	HelloImGui::ImGuiDefaultSettings::LoadDefaultFont_WithFontAwesomeIcons(); // The font that is loaded first is the default font
	gCustomFont = HelloImGui::LoadFontDpiResponsive("fonts/Akronim-Regular.ttf", 40.f); // will be loaded from the assets folder
}


int main(int , char *[]) {
    HelloImGui::RunnerParams params;
    params.appWindowParams.windowGeometry.size = {1280, 720};
    params.appWindowParams.windowTitle = "Dear ImGui example with 'Hello ImGui'";
    params.imGuiWindowParams.defaultImGuiWindowType = HelloImGui::DefaultImGuiWindowType::NoDefaultWindow;

    // Fonts need to be loaded at the appropriate moment during initialization (fonts - part 2/3)
    params.callbacks.LoadAdditionalFonts = MyLoadFonts; // LoadAdditionalFonts is a callback that we set with our own font loading function

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;

    params.callbacks.ShowGui = [&]() {

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            // Demo custom font usage (fonts - part 3/3)
            ImGui::PushFont(gCustomFont->font);
            ImGui::Text("Custom font");
            ImGui::PopFont();

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&params.imGuiWindowParams.backgroundColor); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            #ifndef HELLOIMGUI_MOBILEDEVICE
            if (ImGui::Button("Quit"))
                params.appShallExit = true;
            #endif
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }
    };

    params.imGuiWindowParams.showMenuBar = true;
    params.imGuiWindowParams.showMenu_App = false;
    HelloImGui::Run(params);
    return 0;
}
