#pragma once

/**
@@md#HelloImGui::Dpi

 ### Note about DPI handling, i.e. how to handle screen with high pixel density

 - under windows and linux, with GLFW:

@@md
*/

namespace HelloImGui
{
    float DpiWindowFactor();
    float DpiFontLoadingFactor();
}