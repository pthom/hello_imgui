## Rendering GUI in a DPI independent way


## With HelloImGui

Read documentation inside [hello_imgui/dpi_aware.h](../src/hello_imgui/dpi_aware.h)

## Note about bare ImGui applications

### Windows + Glfw

Glfw are dpi aware by default. If you run a default example application, several issues may arise:

* The window size will be very small: this can be solved by calling `glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);`
* Fonts size will be very small: this can be solved by multiplying the font size at loading time like this: 
```
float xscale, yscale;
glfwGetWindowContentScale((GLFWwindow *) window, &xscale, &yscale);
float fontLoadingScale = xscale / ImGui::FontGlobalScale;
ImGui::GetIO().Fonts->AddFont...(..., fontSize * fontLoadingScale, ...) 
```
* Items will be misplaced if you position them with constant values. Always multiply those values by `ImGui::GetFontSize()`.


### Windows + SDL

SDL does not enable DPI aw  awareness by default (glfw does it by default under windows).

#### How to make SDL application DPI aware:
You can use `ImGui_ImplWin32_EnableDpiAwareness()` from `imgui/examples/application/source/imgui_impl_win32.cpp`
This will make the application dpi aware.

#### How to get a window dpi scale:
Unfortunately, ImGui_ImplWin32_GetDpiScaleForMonitor() from `imgui/examples/application/source/imgui_impl_win32.cpp`
is probably broken.
Note: the stock implementation to get per window dpi from imgui_impl_win32.cpp is broken (ImGui_ImplWin32_GetDpiScaleForHwnd): 
it uses GetDpiForMonitor which does not work for DPI aware threads (see https://learn.microsoft.com/en-us/windows/win32/api/shellscalingapi/nf-shellscalingapi-getdpiformonitor).
As a result, it may return 1, even on high dpi monitor.

A workaround is to use GetDpiForWindow from the win32 api (https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdpiforwindow)
```cpp
    #ifdef _WIN32
    HWND SdlWindowToHwnd(void* sdlwindow) 
    {
        HWND win32Window;
        SDL_SysWMinfo info;
        SDL_VERSION(&info.version);
        bool success = SDL_GetWindowWMInfo(sdlwindow, &info);
        return info.info.win.window;
    }
    
    float SdlWindowDpiScale(void* sdlwindow)
    {
        int dpi = GetDpiForWindow(SdlWindowToHwnd(window));
          float dpiScale = dpi / 96.f;
          return dpiScale;
    }
    #endif
```

