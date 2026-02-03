#include "hello_imgui/hello_imgui.h"
#include <implot.h>
int main(int , char *[])
{
    float a[]= {1.0,2,3};
    float b[]= {1.0,2,3};
    HelloImGui::Run(
        [&a, &b]
        {
            static bool init = false;
            if (!init)
            {
                ImPlot::CreateContext();
                init = true;
            }
            ImGui::Text("Hello, world!");
            if (ImPlot::BeginPlot("Plot"))
            {
                ImPlot::PlotLine("Line", a, b, 3);
                ImPlot::EndPlot();
            }

            if (ImPlot::BeginPlot("Plot 2 "))
            {
                ImPlot::PlotLine("Line 2", a, b, 3);
                ImPlot::EndPlot();
            }

        }, // Gui code
        "Hello!",                            // Window title
        true                                 // Window size auto
    );
    return 0;
}
