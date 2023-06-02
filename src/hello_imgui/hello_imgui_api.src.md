[TOC]

# API

## HelloImGui::Run()

See [hello_imgui.h](hello_imgui.h).

@import "hello_imgui.h" {md_id=HelloImGui::Run}

## Runner params

See [runner_params.h](runner_params.h).

#### Diagram

_RunnerParams_ contains all the settings and callbacks in order to run an application. 
The diagram below summarize all the possible settings and callbacks (which are explained in detail later in this document).

![a](doc_src/hello_imgui_diagram.png)

#### RunnerParams

##### Simple runner params
@import "runner_params.h" {md_id=SimpleRunnerParams}

##### Full params
@import "runner_params.h" {md_id=RunnerParams}

#### Fps Idling
@import "runner_params.h" {md_id=FpsIdling}

See [runner_params.h](runner_params.h).


----

## Runner callbacks

See [runner_callbacks.h](runner_callbacks.h).

#### RunnerCallbacks

@import "runner_callbacks.h" {md_id=RunnerCallbacks}

@import "runner_callbacks.h" {md_id=VoidFunction_AnyEventCallback}

#### MobileCallbacks

@import "runner_callbacks.h" {md_id=MobileCallbacks}

----

## Application window params

See [app_window_params.h](app_window_params.h).

### AppWindowParams

@import "app_window_params.h" {md_id=AppWindowParams}

### WindowGeometry

@import "app_window_params.h" {md_id=WindowGeometry}

----

## ImGui window params

See [imgui_window_params.h](imgui_window_params.h).

#### ImGuiWindowParams

@import "imgui_window_params.h" {md_id=ImGuiWindowParams}

#### Default window types

@import "imgui_window_params.h" {md_id=DefaultImGuiWindowType}

----

## Docking

See [docking_params.h](docking_params.h).

@import "docking_params.h" {md_id=DockingIntro}

#### Docking Params: Example usage

@import "docking_params.h" {md_id=DockingExample}

#### Docking Splits

@import "docking_params.h" {md_id=DockingSplit}

#### Dockable window

@import "docking_params.h" {md_id=DockableWindow}

#### Docking Params

@import "docking_params.h" {md_id=DockingParams}

---

## Handling screen with high DPI: items positioning and font loading

@import "dpi_aware.h" {md_id=Dpi}

----

## Applications assets

See [hello_imgui_assets.h](hello_imgui_assets.h).

#### Assets Files structure

@import "hello_imgui_assets.h" {md_id=AssetsStructure}

#### Load Assets as data buffer

@import "hello_imgui_assets.h" {md_id=LoadAssetFileData}

#### Get assets path

@import "hello_imgui_assets.h" {md_id=assetFileFullPath}

----

## Display images

#### HelloImGui::ImageFromAsset

See [image_from_asset.h](image_from_asset.h).

@import "image_from_asset.h" {md_id=HelloImGui::ImageFromAsset}

## Backend Pointers

@import "backend_pointers.h" {md_id=BackendPointers}

## Store user settings in the ini file

See [hello_imgui.h](hello_imgui.h).

@import "hello_imgui.h" {md_id=HelloImGui::UserPref}

## Switch between several layouts

See [hello_imgui.h](hello_imgui.h).

@import "hello_imgui.h" {md_id=HelloImGui::Layouts}


----
