#pragma once
#include "imgui.h"

namespace HelloImGui
{
// @@md#DpiAwareParams

//
// Hello ImGui will try its best to automatically handle DPI scaling for you.
// It does this by setting two values:
//
// - `dpiWindowSizeFactor`:
//        factor by which window size should be multiplied
//
// - `fontRenderingScale`:
//     factor by which fonts glyphs should be scaled at rendering time
//     (typically 1 on windows, and 0.5 on macOS retina screens)
//
//    By default, Hello ImGui will compute them automatically,
//    when dpiWindowSizeFactor and fontRenderingScale are set to 0.
//
// How to set those values manually:
// ---------------------------------
// If it fails (i.e. your window and/or fonts are too big or too small),
// you may set them manually:
//    (1) Either by setting them programmatically in your application
//        (set their values in `runnerParams.dpiAwareParams`)
//    (2) Either by setting them in a `hello_imgui.ini` file in the current folder, or any of its parent folders.
//       (this is useful when you want to set them for a specific app or set of apps, without modifying the app code)
// Note: if several methods are used, the order of priority is (1) > (2)
//
// Example content of a ini file:
// ------------------------------
//     [DpiAwareParams]
//     dpiWindowSizeFactor=2
//     fontRenderingScale=0.5
//
struct DpiAwareParams
{
    // `dpiWindowSizeFactor`
    //        factor by which window size should be multiplied to get a similar
    //        visible size on different OSes.
    //  In a standard environment (i.e. outside of Hello ImGui), an application with a size of 960x480 pixels,
    //  may have a physical size (in mm or inches) that varies depending on the screen DPI, and the OS.
    //
    //  Inside Hello ImGui, the window size is always treated as targeting a 96 PPI screen, so that its size will
    //  look similar whatever the OS and the screen DPI.
    //  In our example, our 960x480 pixels window will try to correspond to a 10x5 inches window
    //
    //  Hello ImGui does its best to compute it on all OSes.
    //  However, if it fails you may set its value manually.
    //  If it is set to 0, Hello ImGui will compute it automatically,
    //  and the resulting value will be stored in `dpiWindowSizeFactor`.
    float dpiWindowSizeFactor = 0.0f;

    // `fontRenderingScale`
    //     factor (that is either 1 or < 1.) by which fonts glyphs should be
    //     scaled at rendering time.
    //     On macOS retina screens, it will be 0.5, since macOS APIs hide
    //     the real resolution of the screen.
    float fontRenderingScale = 0.0f;

    // `dpiFontLoadingFactor`
    //      factor by which font size should be multiplied at loading time to get a similar
    //      visible size on different OSes.
    //      The size will be equivalent to a size given for a 96 PPI screen
    float DpiFontLoadingFactor() { return dpiWindowSizeFactor / fontRenderingScale;};
};

// ----------------------------------------------------------------------------

// @@md

/**
@@md#DocEmToVec2

Special care must be taken in order to correctly handle screen with high DPI
 (for example, almost all recent laptops screens).

Using ImVec2 with fixed values is *almost always a bad idea* if you intend your
application to be used on high DPI screens!
Otherwise, widgets might be misplaced or too small on different screens and/or OS.

Instead you should use scale your widgets and windows relatively to the font size,
as is done with the [em CSS Unit](https://lyty.dev/css/css-unit.html).

@@md
**/

// @@md#EmToVec2
//  __HelloImGui::EmToVec2()__ returns an ImVec2 that you can use to size
//  or place your widgets in a DPI independent way.
//  Values are in multiples of the font size (i.e. as in the em CSS unit).
ImVec2 EmToVec2(float x, float y);
ImVec2 EmToVec2(ImVec2 v);

// __HelloImGui::EmSize()__ returns the visible font size on the screen.
float EmSize();
// __HelloImGui::EmSize(nbLines)__ returns a size corresponding to nbLines text lines
float EmSize(float nbLines);
// @@md

} // namespace HelloImGui


// ----------------------------------------------------------------------------

//
// Legacy API, you should use RunnerParams.dpAwareParams instead
//
namespace HelloImGui
{
// Multiply font sizes by this factor when loading fonts manually with ImGui::GetIO().Fonts->AddFont...
// (HelloImGui::LoadFontTTF does this by default)
float DpiFontLoadingFactor();

// DpiWindowSizeFactor() is the factor by which window size should be multiplied to get a similar visible size on different OSes.
// It returns ApplicationScreenPixelPerInch / 96  under windows and linux. Under macOS, it will return 1.
float DpiWindowSizeFactor();

// returns the default value that should be stored inside `ImGui::GetIO().FontGlobalScale`
float ImGuiDefaultFontGlobalScale();
} // namespace HelloImGui

