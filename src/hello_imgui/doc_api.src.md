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

# Additional Widgets

## InputTextResizable

```cpp
@import "hello_imgui_widgets.h" {md_id=InputTextResizable}
```

## WidgetWithResizeHandle

```cpp
@import "hello_imgui_widgets.h" {md_id=WidgetWithResizeHandle}
``` 

---

# Handling screens with high DPI

@import "dpi_aware.h" {md_id=HandlingScreenHighDPI}
