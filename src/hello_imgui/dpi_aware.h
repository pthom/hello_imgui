#pragma once

/**
@@md#HelloImGui::Dpi

# Handling screen with high DPI: font loading and items positioning:
--------------------------------------------------------------------

Special care must be taken in order to correctly handle screen with high DPI (e.g. almost all recent laptops screens).

For example, let's consider screen whose physical pixel resolution is 3600x2000
but which will displayed with a scaling factor of 200%, so that widgets do not look too small on it.
The way it is handled depends on the OS.

    - On MacOS, the screen will be seen as having a resolution of 1800x1000, and the OS handles the resizing by itself
    - On Linux, and on Windows if the application is DPI aware, the screen will be seen as having a resolution of 3600x2000
    - On Windows if the application is not DPI aware, the screen will be seen as having a resolution of 1800x1000

By default, if using the glfw backend, applications will be Dpi aware under windows.
Sdl applications are normally *not* Dpi aware. However when HelloImGui uses the SDL backend, it will still be Dpi aware.


## How to load fonts for a crisp font rendering and a correct size:

HelloImGui provides `HelloImGui::DpiFontLoadingFactor()` which corresponds to:
    `DpiWindowFactor() * 1.f / ImGui::GetIO().FontGlobalScale`
              where DpiWindowFactor() is equal to `CurrentScreenPixelPerInch / 96`

==> When loading fonts, multiply their size by this factor!


## How to position widgets on a window in a Dpi independent way

Using ImVec2 with fixed values is almost always a bad idea if you intend your application to be used on high DPI screens.

* Either multiply those values by ImGui::GetFontSize()
* Or use `HelloImGui::EmToVec2(x, y)` which will do this multiplication for you. Em stand for the `em` measurements,
   as used in CSS: 1em simply correspond to the current font height.

@@md
*/

namespace HelloImGui
{
    // Multiply font sizes by this factor when loading fonts manually with ImGui::GetIO().Fonts->AddFont...
    // (HelloImGui::LoadFontTTF does this by default)
    float DpiFontLoadingFactor();

    // DpiWindowFactor() returns ApplicationScreenPixelPerInch / 96  under windows and linux.
    // Under macOS, it will return 1.
    float DpiWindowFactor();
}