#include "hello_imgui/hello_imgui.h"

void Gui()
{
    ImGui::Text("Hello...");
    if (ImGui::Button("Goodbye!"))
        HelloImGui::GetRunnerParams()->appShallExit = true;
};


int main(int , char *[])
{
    // Run 2 applications, one after the other
    // to make sure that the manual render is working correctly and releasing resources
    for (int i = 0; i < 2; i++)
    {
        HelloImGui::ManualRender::SetupFromGuiFunction(Gui);
        while (! HelloImGui::GetRunnerParams()->appShallExit)
        {
            HelloImGui::ManualRender::Render();
        }
        HelloImGui::ManualRender::TearDown();
    }
}
