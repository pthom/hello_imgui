#include "hello_imgui/hello_imgui.h"
int main(int , char *[])
{
    HelloImGui::Run(
        []{ ImGui::Text("Hello, world!"); }, // Gui code
        "Hello!",                            // Window title
        true                                 // Window size auto
    );
    return 0;
}
