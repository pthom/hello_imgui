#include "hello_imgui/hello_imgui.h"
int main(int , char *[])
{
    HelloImGui::Run_AutoSize(
        []{ ImGui::Text("Hello, world!"); }, // Gui code
        "Hello!" );                          // Window title
    return 0;
}
