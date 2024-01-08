# Application parameters

_RunnerParams_ contains all the settings and callbacks in order to run an application. 
The diagram below summarize all the possible settings and callbacks (which are explained in detail later in this document).

[![diagram](https://raw.githubusercontent.com/pthom/hello_imgui/master/src/hello_imgui/doc_src/hello_imgui_diagram.png)](https://raw.githubusercontent.com/pthom/hello_imgui/master/src/hello_imgui/doc_src/hello_imgui_diagram.png)

# RunnerParams

See [runner_params.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/runner_params.h).


## Simple runner params

```cpp
@import "runner_params.h" {md_id=SimpleRunnerParams}
```

## Full params

```cpp
@import "runner_params.h" {md_id=RunnerParams}
```


# Fps Idling

See [runner_params.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/runner_params.h).

```cpp
@import "runner_params.h" {md_id=FpsIdling}
```

----

# Runner callbacks

See [runner_callbacks.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/runner_callbacks.h).

```cpp
@import "runner_callbacks.h" {md_id=RunnerCallbacks}
```

@import "runner_callbacks.h" {md_id=VoidFunction_AnyEventCallback}

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

# Backend Pointers

```cpp
@import "backend_pointers.h" {md_id=BackendPointers}
```
