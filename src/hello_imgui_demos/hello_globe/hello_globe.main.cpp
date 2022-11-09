#include "hello_imgui/hello_imgui.h"

int main(int , char *[])
{
    HelloImGui::SimpleRunnerParams runnerParams;      // runnerParams will contains all the application params and callbacks
    runnerParams.guiFunction =                        // guiFunction contains a lambda function with the Gui code
        [&runnerParams]{

            ImGui::Text("Hello, ");                    // Display a simple label
            HelloImGui::ImageFromAsset("world.jpg");   // Display a static image, taken from assets/world.jpg,
                                                       // assets are embedded automatically into the app (for *all* platforms)

            if (ImGui::Button("Bye!"))                 // Display a button
                // ... and immediately handle its action if it is clicked!
                HelloImGui::GetRunnerParams()->appShallExit = true;

            float fps = ImGui::GetIO().Framerate;
            ImGui::Text("FPS: %.2f", fps);
        };

    runnerParams.windowTitle = "Hello, globe!";
    runnerParams.windowSize = {180, 220};
    runnerParams.windowRestorePreviousGeometry = false;
    HelloImGui::Run(runnerParams);
    return 0;
}
