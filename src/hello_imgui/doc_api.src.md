# API

# Run Application

HelloImGui is extremely easy to use: there is **one** main function in the API, with three overloads.

![[hello_imgui.h#HelloImGui::Run]]

# Run Application while handling the rendering loop
If you want to be in control of the rendering loop, you may use the namespace `HelloImGui::ManualRender` (available since September 2024)

![[hello_imgui.h#HelloImGui::ManualRender]]

----

# Place widgets in a DPI-aware way

![[dpi_aware.h#DocEmToVec2]]

![[dpi_aware.h#EmToVec2]]

----
# Load fonts
See [hello_imgui_font.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/hello_imgui_font.h).
![[hello_imgui_font.h#Fonts]]

----

# Applications assets
See [hello_imgui_assets.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/hello_imgui_assets.h).

## Load Assets as data buffer

![[hello_imgui_assets.h#LoadAssetFileData]]

## Get assets path

![[hello_imgui_assets.h#assetFileFullPath]]

## Set assets folder

![[hello_imgui_assets.h#AssetsSearchPaths]]


## Display images from assets
See [image_from_asset.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/image_from_asset.h).
![[image_from_asset.h#HelloImGui::ImageFromAsset]]

----

# Utility functions

![[hello_imgui.h#UtilityFunctions]]

----
# Switch between several layouts
See [hello_imgui.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/hello_imgui.h).
    
![[hello_imgui.h#HelloImGui::Layouts]]

----

# Ini settings

## Ini settings location

![[runner_params.h#IniFolderType]]

![[runner_params.h#IniIniSettingsLocation]]

----

## Store user settings in the ini file
See [hello_imgui.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/hello_imgui.h).

![[hello_imgui.h#HelloImGui::UserPref]]

----

# Customize Hello ImGui Menus

![[hello_imgui.h#MenuIntro]]

![[hello_imgui.h#MenuFunctions]]

---

# Additional Widgets

## InputTextResizable

![[hello_imgui_widgets.h#InputTextResizable]]

## WidgetWithResizeHandle

![[hello_imgui_widgets.h#WidgetWithResizeHandle]]

---

# Handling screens with high DPI

![[dpi_aware.h#HandlingScreenHighDPI]]
