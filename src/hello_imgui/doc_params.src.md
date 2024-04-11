# Application parameters

@import "hello_imgui.h" {md_id=HelloImGui::Run}

# Diagram

The diagram below summarize all the possible settings and callbacks (which are explained in detail later in this document).

[![diagram](https://raw.githubusercontent.com/pthom/hello_imgui/master/src/hello_imgui/doc_src/hello_imgui_diagram.jpg)](https://raw.githubusercontent.com/pthom/hello_imgui/master/src/hello_imgui/doc_src/hello_imgui_diagram.jpg)

# RunnerParams

See [runner_params.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/runner_params.h).


## Simple runner params

```cpp
@import "runner_params.h" {md_id=SimpleRunnerParams}
```

---

## Full params

```cpp
@import "runner_params.h" {md_id=RunnerParams}
```

### Backend selection


```cpp
@import "runner_params.h" {md_id=PlatformBackendType}
```


# Runner callbacks

See [runner_callbacks.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/runner_callbacks.h).

## Callbacks types
```cpp
@import "runner_callbacks.h" {md_id=VoidFunction_AnyEventCallback}
```

## RunnerCallbacks
```cpp
@import "runner_callbacks.h" {md_id=RunnerCallbacks}
```


## Edge Toolbars Callbacks
More details on `RunnerParams.edgesToolbars` (a dictionary of `EdgeToolbar`, per edge type)

```cpp
struct RunnerCallbacks
{
    ...
    // EdgesToolbars: A map that contains the definition of toolbars
    // that can be placed on the edges of the App window
    std::map<EdgeToolbarType, EdgeToolbar> edgesToolbars;
    void AddEdgeToolbar(EdgeToolbarType edgeToolbarType,
                        VoidFunction callback,
                        const EdgeToolbarOptions& options = EdgeToolbarOptions());
    ...
};
```

Where:
```cpp
@import "runner_callbacks.h" {md_id=EdgeToolbar}
```

## MobileCallbacks

```cpp
@import "runner_callbacks.h" {md_id=MobileCallbacks}
```

----

# Application window params

See [app_window_params.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/app_window_params.h).

## AppWindowParams

```cpp
@import "app_window_params.h" {md_id=AppWindowParams}
```

## WindowGeometry

```cpp
@import "app_window_params.h" {md_id=WindowGeometry}
```
----

# ImGui window params

See [imgui_window_params.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/imgui_window_params.h).

## ImGuiWindowParams

```cpp
@import "imgui_window_params.h" {md_id=ImGuiWindowParams}
```

## Default window types

```cpp
@import "imgui_window_params.h" {md_id=DefaultImGuiWindowType}
```

# Fps Idling

See [runner_params.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/runner_params.h).

```cpp
@import "runner_params.h" {md_id=FpsIdling}
```

# Dpi Aware Params

Optionally, DPI parameters can be fine-tuned. For detailed info, see [handling screens with high dpi](https://pthom.github.io/hello_imgui/book/doc_api.html#handling-screens-with-high-dpi)

Source: [dpi_aware.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/dpi_aware.h)
```cpp
@import "dpi_aware.h" {md_id=DpiAwareParams}
```


----

# Docking

See [docking_params.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/docking_params.h).

@import "docking_params.h" {md_id=DockingIntro}

## Docking Params: Example usage

@import "docking_params.h" {md_id=DockingExample}

## Docking Splits

```cpp
@import "docking_params.h" {md_id=DockingSplit}
```

## Dockable window

```cpp
@import "docking_params.h" {md_id=DockableWindow}
```

## Docking Params

```cpp
@import "docking_params.h" {md_id=DockingParams}
```

# Backend

## Backend Pointers

```cpp
@import "backend_pointers.h" {md_id=BackendPointers}
```

## Renderer Backend Options

```cpp
@import "renderer_backend_options.h" {md_id=RendererBackendOptions}
```
