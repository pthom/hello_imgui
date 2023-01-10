#ifdef _WIN32

// Adapted from imgui/backends/imgui_impl_win32.cpp
// Use this to enable DPI awareness without having to create an application manifest.
// This is useful for SDL. Glfw does this by itself.
namespace HelloImGui 
{
namespace Internal 
{
    void ImGui_ImplWin32_EnableDpiAwareness();

    // Note: the stock implementation to get per window dpifrom imgui_impl_win32.cpp is broken:
    // it uses GetDpiForMonitor which does not work for DPI aware threads
    // (see https://learn.microsoft.com/en-us/windows/win32/api/shellscalingapi/nf-shellscalingapi-getdpiformonitor)
    //     float ImGui_ImplWin32_GetDpiScaleForHwnd(void* hwnd);
    //     float ImGui_ImplWin32_GetDpiScaleForMonitor(void* monitor);
    //
    //  ==> instead, we use Win32's GetDpiForWindow for the sdl backend
}
}
#endif
