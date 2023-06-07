#include "hello_imgui/hello_imgui.h"

int main(int , char *[])
{
    auto guiFunction = []() {
        HelloImGui::ImageFromAsset("moon.jpg");          // Display a static image
        HelloImGui::ImageFromAsset("moon.jpg", false);   // Display a static image
        HelloImGui::ImageFromAsset("cpp.gif", true);     // Display a GIF image
        HelloImGui::ImageButtonFromAsset("py.gif", true);// Display a GIF button
     };
    HelloImGui::Run(guiFunction, "Hello, GIF", false);
    return 0;
}
