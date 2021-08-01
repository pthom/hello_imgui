#include "hello_imgui/hello_imgui.h"
int main(int , char *[])
{
    HelloImGui::RunnerParams runnerParams;
    runnerParams.callbacks.ShowGui =            // ShowGui contains a lambda function with the Gui code
        [&]{
            ImGui::Text("Hello, ");                    // Display a simple label
            HelloImGui::ImageFromAsset("world.jpg");   // Display a static image, taken from assets/world.jpg,
                                                       // assets are embedded automatically into the app (for *all* platforms)
            if (ImGui::Button("Bye!"))                 // Display a button, and immediately handle its action if clicked!
                runnerParams.appShallExit = true;
        };
    runnerParams.appWindowParams.windowTitle = "Hello, globe!";
    runnerParams.appWindowParams.windowSize = {180.f, 210.f};
    HelloImGui::Run(runnerParams);
    return 0;
}
