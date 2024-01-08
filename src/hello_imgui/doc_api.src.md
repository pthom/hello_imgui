# API

# Run Application

HelloImGui is extremely easy to use: there is **one** main function in the API, with three overloads.

@import "hello_imgui.h" {md_id=HelloImGui::Run}

----

# Place widgets in a DPI-aware way

@import "dpi_aware.h" {md_id=DocEmToVec2}

```cpp
@import "dpi_aware.h" {md_id=EmToVec2}
```

----
# Load fonts
See [hello_imgui_font.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/hello_imgui_font.h).
```cpp
@import "hello_imgui_font.h" {md_id=Fonts}
```

----

# Applications assets
See [hello_imgui_assets.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/hello_imgui_assets.h).

## Load Assets as data buffer

```cpp
@import "hello_imgui_assets.h" {md_id=LoadAssetFileData}
```

## Get assets path

```cpp
@import "hello_imgui_assets.h" {md_id=assetFileFullPath}
```


## Display images from assets
See [image_from_asset.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/image_from_asset.h).
```cpp
@import "image_from_asset.h" {md_id=HelloImGui::ImageFromAsset}
```

----

# Utility functions

```cpp
@import "hello_imgui.h" {md_id=UtilityFunctions}
```

----
# Switch between several layouts
See [hello_imgui.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/hello_imgui.h).
    
```cpp
@import "hello_imgui.h" {md_id=HelloImGui::Layouts}
```

----

# Ini settings

## Ini settings location

```cpp
@import "runner_params.h" {md_id=IniFolderType}
```

```cpp
@import "runner_params.h" {md_id=IniIniSettingsLocation}
```

----

## Store user settings in the ini file
See [hello_imgui.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/hello_imgui.h).

```cpp
@import "hello_imgui.h" {md_id=HelloImGui::UserPref}
```

----

# Customize Hello ImGui Menus

@import "hello_imgui.h" {md_id=MenuIntro}

```cpp
@import "hello_imgui.h" {md_id=MenuFunctions}
```

---

# Handling screens with high DPI

_Note: This part is relevant only for more advanced usages. If you use `HelloImGui::LoadFont()`, and always use `HelloImGui::EmToVec2()` to place widgets, you do not need to worry about DPI handling_ 

## Details on DPI handling on different OS

Let's consider screen whose physical pixel resolution is 3600x2000, but which will displayed with a scaling factor of 200%, so that widgets do not look too small on it.

The way it is handled depends on the OS:
- On MacOS, the screen will be seen as having a resolution of 1800x1000, and the OS handles the resizing by itself.
- On Linux, and on Windows if the application is DPI aware, the screen will be seen as having a resolution of 3600x2000.
- On Windows if the application is not DPI aware, the screen will be seen as having a resolution of 1800x1000

By default, if using the glfw backend, applications will be Dpi aware under windows.
Sdl applications are normally not Dpi aware. However HelloImGui makes them Dpi aware when using the sdl backend.


## Dpi aware Font scaling

`HelloImGui::LoadFont()` will load fonts with the correct size, taking into account the DPI scaling.

If you prefer to use `ImGui::GetIO().Fonts->AddFontFromFileTTF()`, there are two things to know:

1. You should adjust `ImGui::GetIO().FontGlobalScale`:

Under windows and linux, it should be is 1: no rescaling should be done by ImGui.
Under macOS and emscripten, it may need to bet set to 0.5 (for example it will be 0.5 if the dpi scaling is 200%
on a macOS retina screen)

`HelloImGui::ImGuiDefaultFontGlobalScale()` returns the default value that should be stored inside `ImGui::GetIO().FontGlobalScale`.


2. You should adjust the font size when loading a font:

`HelloImGui::DpiFontLoadingFactor()` returns a factor by which you shall multiply your font sizes when loading them.

`HelloImGui::DpiFontLoadingFactor()` corresponds to:
`DpiWindowSizeFactor() * 1.f / ImGui::GetIO().FontGlobalScale`

where DpiWindowSizeFactor() is equal to `CurrentScreenPixelPerInch / 96`
under windows and linux, 1 under macOS
