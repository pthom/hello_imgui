#pragma once
#include "imgui.h"

/**
@@md#Dpi

Special care must be taken in order to correctly handle screen with high DPI (for example, almost all recent laptops screens).
Otherwise, widgets might be misplaced or too small, and font rendering might be blurry or too small.

### How to position widgets on a window in a Dpi independent way

Using ImVec2 with fixed values is *almost always a bad idea* if you intend your application to be used on high DPI screens.
Instead you can:
* either multiply those values by ImGui::GetFontSize()
* or use `HelloImGui::EmToVec2(x, y)` which will do this multiplication for you. Em stand for the `em` measurements,
   as used in CSS: 1em simply correspond to the current font height.


### How to load fonts for a crisp font rendering and a correct size

HelloImGui provides `HelloImGui::DpiFontLoadingFactor()` which corresponds to:
    `DpiWindowSizeFactor() * 1.f / ImGui::GetIO().FontGlobalScale`
              where DpiWindowSizeFactor() is equal to `CurrentScreenPixelPerInch / 96` under windows and linux, 1 under macOS

==> When loading fonts, multiply their size by this factor!

### More details on DPI handling with different OS and backends

Let's consider screen whose physical pixel resolution is 3600x2000, but which will displayed with a scaling factor of 200%,
 so that widgets do not look too small on it.

The way it is handled depends on the OS:
- On MacOS, the screen will be seen as having a resolution of 1800x1000, and the OS handles the resizing by itself.
- On Linux, and on Windows if the application is DPI aware, the screen will be seen as having a resolution of 3600x2000.
- On Windows if the application is not DPI aware, the screen will be seen as having a resolution of 1800x1000

By default, if using the glfw backend, applications will be Dpi aware under windows.
Sdl applications are normally not Dpi aware. However HelloImGui makes them Dpi aware when using the sdl backend.


### HelloImGui Dpi aware C++ API

`HelloImGui::EmSize()` (C++) and `hello_imgui.em_size()` (Python) return the visible font size on the screen.
For reproducible results, even on HighDPI screens, always scale your widgets and windows relatively to this size.
 It is somewhat comparable to the [em CSS Unit](https://lyty.dev/css/css-unit.html).

`HelloImGui::EmToVec2(x, y)` (C++) and `hello_imgui.em_to_vec2(x,y)` (Python) return an ImVec2 that you can use
 to size or place your widgets in a DPI independent way.

`HelloImGui::EmSize(nbLines)` (C++) and `hello_imgui.em_size(nb_lines)` (Python) return a size corresponding to nbLines text lines

`HelloImGui::DpiFontLoadingFactor()` (C++) and `hello_imgui.dpi_font_loading_factor()` (Python) return a factor by
 which you shall multiply your font sizes when loading fonts manually with _ImGui::GetIO().Fonts->AddFont..._
 HelloImGui::LoadFontTTF does this by default.

`HelloImGui::ImGuiDefaultFontGlobalScale()` (C++) and `hello_imgui.imgui_default_font_global_scale()` (Python) returns the
 default value that should be stored inside `ImGui::GetIO().FontGlobalScale`.
 Under windows and linux, this is always 1: no rescaling should be done by ImGui. Under macOS and emscripten,
 this can be < 1 (for example it will be 0.5 if the dpi scaling is 200%)
@@md
*/

namespace HelloImGui
{
    // __HelloImGui::EmSize()__ returns the visible font size on the screen. For good results on HighDPI screens, always scale your
    // widgets and windows relatively to this size.
    // It is somewhat comparable to the [em CSS Unit](https://lyty.dev/css/css-unit.html).
    // EmSize() = ImGui::GetFontSize()
    float EmSize();

    // __HelloImGui::EmSize(nbLines)__ returns a size corresponding to nbLines text lines
    float EmSize(float nbLines);

    // __HelloImGui::EmToVec2()__ returns an ImVec2 that you can use to size or place your widgets in a DPI independent way
    ImVec2 EmToVec2(float x, float y);
    ImVec2 EmToVec2(ImVec2 v);

    // Multiply font sizes by this factor when loading fonts manually with ImGui::GetIO().Fonts->AddFont...
    // (HelloImGui::LoadFontTTF does this by default)
    float DpiFontLoadingFactor();

    // DpiWindowSizeFactor() is the factor by which window size should be multiplied to get a similar visible size on different OSes.
    // It returns ApplicationScreenPixelPerInch / 96  under windows and linux. Under macOS, it will return 1.
    float DpiWindowSizeFactor();

    // returns the default value that should be stored inside `ImGui::GetIO().FontGlobalScale`
    float ImGuiDefaultFontGlobalScale();
}