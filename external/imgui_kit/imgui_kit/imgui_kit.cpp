// imgui_kit: small, standalone additions to Dear ImGui
// https://github.com/pthom/imgui_kit (MIT license)

/*

Index of this file:
// [SECTION] Em units

*/

#include "imgui_kit.h"

namespace ImGuiKit
{

//-----------------------------------------------------------------------------
// [SECTION] Em units
//-----------------------------------------------------------------------------

float EmSize()
{
    IM_ASSERT(ImGui::GetCurrentContext() != nullptr && "EmSize() needs a Dear ImGui context");
    return ImGui::GetFontSize();
}

float EmSize(float nb_lines)
{
    return EmSize() * nb_lines;
}

ImVec2 EmToVec2(float x, float y)
{
    float k = EmSize();
    return ImVec2(k * x, k * y);
}

ImVec2 EmToVec2(ImVec2 v)
{
    return EmToVec2(v.x, v.y);
}

ImVec2 PixelsToEm(ImVec2 pixels)
{
    float k = EmSize();
    return ImVec2(pixels.x / k, pixels.y / k);
}

float PixelSizeToEm(float pixel_size)
{
    return pixel_size / EmSize();
}

} // namespace ImGuiKit
