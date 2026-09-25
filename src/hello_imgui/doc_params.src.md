# Application parameters

![[hello_imgui.h#HelloImGui::Run]]

# Diagram

The diagram below summarize all the possible settings and callbacks (which are explained in detail later in this document).

[![diagram](https://raw.githubusercontent.com/pthom/hello_imgui/master/src/hello_imgui/doc_src/hello_imgui_diagram.jpg)](https://raw.githubusercontent.com/pthom/hello_imgui/master/src/hello_imgui/doc_src/hello_imgui_diagram.jpg)

# RunnerParams

See [runner_params.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/runner_params.h).


## Simple runner params

![[runner_params.h#SimpleRunnerParams]]

---

## Full params

![[runner_params.h#RunnerParams]]

### Backend selection


![[runner_params.h#PlatformBackendType]]


# Runner callbacks

See [runner_callbacks.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/runner_callbacks.h).

## Callbacks types
![[runner_callbacks.h#VoidFunction_AnyEventCallback]]

## RunnerCallbacks
![[runner_callbacks.h#RunnerCallbacks]]


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
![[runner_callbacks.h#EdgeToolbar]]

## MobileCallbacks

![[runner_callbacks.h#MobileCallbacks]]

----

# Application window params

See [app_window_params.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/app_window_params.h).

## AppWindowParams

![[app_window_params.h#AppWindowParams]]

## WindowGeometry

![[app_window_params.h#WindowGeometry]]
----

# ImGui window params

See [imgui_window_params.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/imgui_window_params.h).

## ImGuiWindowParams

![[imgui_window_params.h#ImGuiWindowParams]]

## Default window types

![[imgui_window_params.h#DefaultImGuiWindowType]]

# Fps Idling

See [runner_params.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/runner_params.h).

![[runner_params.h#FpsIdling]]

# Dpi Aware Params

Optionally, DPI parameters can be fine-tuned. For detailed info, see [handling screens with high dpi](https://pthom.github.io/hello_imgui/book/doc_api.html#handling-screens-with-high-dpi)

Source: [dpi_aware.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/dpi_aware.h)
![[dpi_aware.h#DpiAwareParams]]


----

# Docking

See [docking_params.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/docking_params.h).

![[docking_params.h#DockingIntro]]

## Docking Params: Example usage

![[docking_params.h#DockingExample]]

## Docking Splits

![[docking_params.h#DockingSplit]]

## Dockable window

![[docking_params.h#DockableWindow]]

## Docking Params

![[docking_params.h#DockingParams]]

# Backend

## Backend Pointers

![[backend_pointers.h#BackendPointers]]

## Renderer Backend Options

![[renderer_backend_options.h#RendererBackendOptions]]
