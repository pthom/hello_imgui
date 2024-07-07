#pragma once
#include "imgui.h"

namespace HelloImGui
{
// @@md#DpiAwareParams

//
// Hello ImGui will try its best to automatically handle DPI scaling for you.
//
// Parameters to change the scaling behavior:
// ------------------------------------------
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
// Parameters to improve font rendering quality:
// ---------------------------------------------
// - `fontOversampleH` and `fontOversampleV` : Font oversampling parameters
//     Rasterize at higher quality for sub-pixel positioning. Probably unused if freeType is used.
//     If not zero, these values will be used to set the oversampling factor when loading fonts.
//
//
// How to set those values manually:
// ---------------------------------
// If it fails (i.e. your window and/or fonts are too big or too small),
// you may set them manually:
//    (1) Either by setting them programmatically in your application
//        (set their values in `runnerParams.dpiAwareParams`)
//    (2) Either by setting them in a `hello_imgui.ini` file. See hello_imgui/hello_imgui_example.ini for more info
// Note: if several methods are used, the order of priority is (1) > (2)
//
// For more information, see the documentation on DPI handling, here: https://pthom.github.io/hello_imgui/book/doc_api.html#handling-screens-with-high-dpi
//
struct DpiAwareParams
{
    // `dpiWindowSizeFactor`
    //     factor by which window size should be multiplied to get a similar
    //     physical size on different OSes (as if they were all displayed on a 96 PPI screen).
    //     This affects the size of native app windows,
    //     but *not* imgui Windows, and *not* the size of widgets and text.
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
    //     factor (that is either 1 or < 1.) by which fonts glyphs should be scaled at rendering time.
    //  On macOS retina screens, it will be 0.5, since macOS APIs hide the real resolution of the screen.
    //  Changing this value will *not* change the visible font size on the screen, however it will
    //  affect the size of the loaded glyphs.
    //  For example, if fontRenderingScale=0.5 (which is the default on a macOS retina screen),
    //  a font size of 16 will be loaded as if it was 32, and will be rendered at half size.
    //   This leads to a better rendering quality on some platforms.
    // (This parameter will be used to set ImGui::GetIO().FontGlobalScale at startup)
    float fontRenderingScale = 0.0f;

    // `onlyUseFontDpiResponsive`
    // If true, guarantees that only HelloImGui::LoadDpiResponsiveFont will be used to load fonts.
    // (also for the default font)
    bool onlyUseFontDpiResponsive = false;

    // `fontOversampleH` and `fontOversampleV` : Font oversampling parameters
    // Rasterize at higher quality for sub-pixel positioning. Probably unused if freeType is used.
    // If not zero, these values will be used to set the oversampling factor when loading fonts.
    // (i.e. they will be set in ImFontConfig::OversampleH and ImFontConfig::OversampleV)
    // OversampleH: The difference between 2 and 3 for OversampleH is minimal.
    //              You can reduce this to 1 for large glyphs save memory.
    // OversampleV: This is not really useful as we don't use sub-pixel positions on the Y axis.
    // Read https://github.com/nothings/stb/blob/master/tests/oversample/README.md for details.
    int             fontOversampleH = 0;  // Default is 2 in ImFontConfig
    int             fontOversampleV = 0;  // Default is 1 in ImFontConfig


    // `dpiFontLoadingFactor`
    //     factor by which font size should be multiplied at loading time to get a similar
    //     visible size on different OSes.
    //  The size will be equivalent to a size given for a 96 PPI screen
    float DpiFontLoadingFactor() const {
        float r = dpiWindowSizeFactor / fontRenderingScale;
        return r;
    };
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

// __HelloImGui::PixelToEm()__ converts a Vec2 in pixels coord to a Vec2 in em units
ImVec2 PixelsToEm(ImVec2 pixels);

// __HelloImGui::PixelSizeToEm()__ converts a size in pixels coord to a size in em units
float  PixelSizeToEm(float pixelSize);

// @@md

// Returns the current DpiAwareParams, which are used
// for font loading and window size scaling
DpiAwareParams* GetDpiAwareParams();

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


// ----------------------------------------------------------------------------
//           Handling screens with high DPI
// ----------------------------------------------------------------------------
/*
@@md#HandlingScreenHighDPI

_Note: This part is relevant only for more advanced usages. If you use `HelloImGui::LoadFont()`,
 and always use `HelloImGui::EmToVec2()` to place widgets, you do not need to worry about DPI handling_

## OS specificities

There are several important things to know about high-DPI handling within Hello ImGui and Dear ImGui:

1. (virtual) screen coordinates vs (physical) pixels
2. DisplayFramebufferScale: Frame buffer size vs window size
3. FontGlobalScale: display-time font scaling factor
4. How to load fonts with the correct size
5. How to get similar window sizes on different OSes/DPI


## Screen coordinates

Screen coordinates are the coordinates you use to place and size windows on the screen.

**Screen coordinates do not always correspond to physical pixels**

- On macOS/iOS retina screens, a screen coordinate corresponds typically
  to 2x2 physical pixels (but this may vary if you change the display scaling)
- On most Linux distributions, whenever there is a high DPI screen
  you can set the display scale. For example if you set the scale to 300%,
  then a screen coordinate will correspond to 3x3 physical pixels
- On Windows, there are two possible situations:
    - If the application is DPI aware, a screen coordinate corresponds to 1x1 physical pixel,
      and you can use the full extent of your screen resolution.
    - If the application is not DPI aware, a screen coordinate may correspond to 2x2 physical pixels
      (if the display scaling is set to 200% for example). However, the rendering of your application
      will be blurry and will not use the full extent of your screen resolution.
    - Notes:
        - Applications created with HelloImGui are DPI aware by default (when using glfw and sdl backends).
        - SDL applications are normally not DPI aware. However, HelloImGui makes them DPI aware.


## DisplayFramebufferScale
`DisplayFramebufferScale` is the ratio between the frame buffer size and the window size.

The frame buffer size is the size of the internal buffer used by the rendering backend.
It might be bigger than the actual window size.
`ImVec2 ImGui::GetIO().DisplayFramebufferScale` is a factor by which the frame buffer size is bigger than the window size.
It is set by the platform backend after it was initialized, and typically reflects the scaling ratio between
physical pixels and screen coordinates.

Under windows, it will always be (1,1). Under macOS / linux, it will reflect the current display scaling.
It will typically be (2,2) on a macOS retina screen.

Notes:
- You cannot change DisplayFramebufferScale manually, it will be reset at each new frame, by asking the platform backend.


## FontGlobalScale

`ImGui::GetIO().FontGlobalScale` is a factor by which fonts glyphs should be scaled at rendering time.
It is typically 1 on windows, and 0.5 on macOS retina screens.


## How to load fonts with the correct size

### Using HelloImGui (recommended)

[`HelloImGui::LoadFont()` and `HelloImGui::LoadFontDpiResponsive`](https://pthom.github.io/hello_imgui/book/doc_api.html#load-fonts) will load fonts
 with the correct size, taking into account the DPI scaling.

### Using Dear ImGui
`ImGui::GetIO().Fonts->AddFontFromFileTTF()` loads a font with a given size, in *physical pixels*.

If for example, DisplayFramebufferScale is (2,2), and you load a font with a size of 16, it will by default be rendered
 with size of 16 *virtual screen coordinate pixels* (i.e. 32 physical pixels). This will lead to blurry text.
To solve this, you should load your font with a size of 16 *virtual screen coordinate pixels* (i.e. 32 physical pixels),
and set `ImGui::GetIO().FontGlobalScale` to 0.5.

Helpers if using `ImGui::GetIO().Fonts->AddFontFromFileTTF()`:
- `HelloImGui::ImGuiDefaultFontGlobalScale()` returns the default value that should be stored inside `ImGui::GetIO().FontGlobalScale`.
- `HelloImGui::DpiFontLoadingFactor()` returns a factor by which you shall multiply your font sizes when loading them.


## Reproducible physical window sizes (in mm or inches)

### Using HelloImGui
Simply specify a window size that corresponds to theoretical 96 PPI screen (inside `RunnerParams.appWindowParams.windowGeometry.size`)

### Using your own code to create the backend window
If you prefer to create the window by yourself, its physical size in millimeters may vary widely,
depending on the OS and the current screen DPI setting.
Typically under Windows, your window may appear to be very small if your screen is high DPI.

To get a similar window size on different OSes/DPI, you should multiply the window size by `HelloImGui::DpiWindowSizeFactor()`.

Note: DpiWindowSizeFactor() is equal to `CurrentScreenPixelPerInch / 96` under windows and linux, and always 1 under macOS.

## Fine tune DPI Handling

See [`HelloImGui::DpiAwareParams`](https://pthom.github.io/hello_imgui/book/doc_params.html#dpi-aware-params)
for more information on how to fine tune DPI handling when using Hello ImGui.
@@md
*/
