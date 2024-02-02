#ifdef _WIN32
#include <cstdio>

#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#define IS_STANDARD_WINDOWS
#else
#define IS_UWP
#endif


// Adapted from imgui/backends/imgui_impl_win32.cpp
#include "imgui.h"
#ifndef __MINGW32__
#include <Windows.h>
#else
#include <windows.h>
#endif

namespace HelloImGui
{
namespace Internal
{
    #ifdef IS_UWP
        void ImGui_ImplWin32_EnableDpiAwareness()
        {
            fprintf(stderr, "ImGui_ImplWin32_EnableDpiAwareness not implemented for __cplusplus_winrt, aka uwp\n");
        }
    #else


    //--------------------------------------------------------------------------------------------------------
    // DPI-related helpers (optional)
    //--------------------------------------------------------------------------------------------------------
    // - Use to enable DPI awareness without having to create an application manifest.
    // - Your own app may already do this via a manifest or explicit calls. This is mostly useful for our
    // examples/ apps.
    // - In theory we could call simple functions from Windows SDK such as SetProcessDPIAware(),
    // SetProcessDpiAwareness(), etc.
    //   but most of the functions provided by Microsoft require Windows 8.1/10+ SDK at compile time and
    //   Windows 8/10+ at runtime, neither we want to require the user to have. So we dynamically select and
    //   load those functions to avoid dependencies.
    //---------------------------------------------------------------------------------------------------------
    // This is the scheme successfully used by GLFW (from which we borrowed some of the code) and other apps
    // aiming to be highly portable. ImGui_ImplWin32_EnableDpiAwareness() is just a helper called by main.cpp,
    // we don't call it automatically. If you are trying to implement your own backend for your own engine,
    // you may ignore that noise.
    //---------------------------------------------------------------------------------------------------------

    // Perform our own check with RtlVerifyVersionInfo() instead of using functions from <VersionHelpers.h> as
    // they require a manifest to be functional for checks above 8.1. See
    // https://github.com/ocornut/imgui/issues/4200
    static BOOL _IsWindowsVersionOrGreater(WORD major, WORD minor, WORD)
    {
        typedef LONG(WINAPI * PFN_RtlVerifyVersionInfo)(OSVERSIONINFOEXW*, ULONG, ULONGLONG);
        static PFN_RtlVerifyVersionInfo RtlVerifyVersionInfoFn = nullptr;
        if (RtlVerifyVersionInfoFn == nullptr)
            if (HMODULE ntdllModule = ::GetModuleHandleA("ntdll.dll"))
                RtlVerifyVersionInfoFn =
                    (PFN_RtlVerifyVersionInfo)GetProcAddress(ntdllModule, "RtlVerifyVersionInfo");
        if (RtlVerifyVersionInfoFn == nullptr)
            return FALSE;

        RTL_OSVERSIONINFOEXW versionInfo = {};
        ULONGLONG conditionMask = 0;
        versionInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
        versionInfo.dwMajorVersion = major;
        versionInfo.dwMinorVersion = minor;
        VER_SET_CONDITION(conditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
        VER_SET_CONDITION(conditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
        return (RtlVerifyVersionInfoFn(&versionInfo, VER_MAJORVERSION | VER_MINORVERSION, conditionMask) == 0)
                   ? TRUE
                   : FALSE;
    }

#define _IsWindowsVistaOrGreater() \
    _IsWindowsVersionOrGreater(HIBYTE(0x0600), LOBYTE(0x0600), 0)  // _WIN32_WINNT_VISTA
#define _IsWindows8OrGreater() \
    _IsWindowsVersionOrGreater(HIBYTE(0x0602), LOBYTE(0x0602), 0)  // _WIN32_WINNT_WIN8
#define _IsWindows8Point1OrGreater() \
    _IsWindowsVersionOrGreater(HIBYTE(0x0603), LOBYTE(0x0603), 0)  // _WIN32_WINNT_WINBLUE
#define _IsWindows10OrGreater() \
    _IsWindowsVersionOrGreater( \
        HIBYTE(0x0A00), LOBYTE(0x0A00), 0)  // _WIN32_WINNT_WINTHRESHOLD / _WIN32_WINNT_WIN10

#ifndef DPI_ENUMS_DECLARED
    typedef enum
    {
        PROCESS_DPI_UNAWARE = 0,
        PROCESS_SYSTEM_DPI_AWARE = 1,
        PROCESS_PER_MONITOR_DPI_AWARE = 2
    } PROCESS_DPI_AWARENESS;
    typedef enum
    {
        MDT_EFFECTIVE_DPI = 0,
        MDT_ANGULAR_DPI = 1,
        MDT_RAW_DPI = 2,
        MDT_DEFAULT = MDT_EFFECTIVE_DPI
    } MONITOR_DPI_TYPE;
#endif
#ifndef _DPI_AWARENESS_CONTEXTS_
    DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE (DPI_AWARENESS_CONTEXT) - 3
#endif
#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 (DPI_AWARENESS_CONTEXT) - 4
#endif
    typedef HRESULT(WINAPI* PFN_SetProcessDpiAwareness)(
        PROCESS_DPI_AWARENESS);  // Shcore.lib + dll, Windows 8.1+
    typedef HRESULT(WINAPI* PFN_GetDpiForMonitor)(HMONITOR,
                                                  MONITOR_DPI_TYPE,
                                                  UINT*,
                                                  UINT*);  // Shcore.lib + dll, Windows 8.1+
    typedef DPI_AWARENESS_CONTEXT(WINAPI* PFN_SetThreadDpiAwarenessContext)(
        DPI_AWARENESS_CONTEXT);  // User32.lib + dll, Windows 10 v1607+ (Creators Update)

    // Helper function to enable DPI awareness without setting up a manifest
    void ImGui_ImplWin32_EnableDpiAwareness()
    {
        // Make sure monitors will be updated with latest correct scaling
        // if (ImGui_ImplWin32_Data* bd = ImGui_ImplWin32_GetBackendData())
        //    bd->WantUpdateMonitors = true;

        if (_IsWindows10OrGreater())
        {
            static HINSTANCE user32_dll = ::LoadLibraryA("user32.dll");  // Reference counted per-process
            if (PFN_SetThreadDpiAwarenessContext SetThreadDpiAwarenessContextFn =
                    (PFN_SetThreadDpiAwarenessContext)::GetProcAddress(user32_dll,
                                                                       "SetThreadDpiAwarenessContext"))
            {
                SetThreadDpiAwarenessContextFn(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
            }
        }
        if (_IsWindows8Point1OrGreater())
        {
            static HINSTANCE shcore_dll = ::LoadLibraryA("shcore.dll");  // Reference counted per-process
            if (PFN_SetProcessDpiAwareness SetProcessDpiAwarenessFn =
                    (PFN_SetProcessDpiAwareness)::GetProcAddress(shcore_dll, "SetProcessDpiAwareness"))
            {
                SetProcessDpiAwarenessFn(PROCESS_PER_MONITOR_DPI_AWARE);
                return;
            }
        }
#if _WIN32_WINNT >= 0x0600
        ::SetProcessDPIAware();
#endif
    }

#if defined(_MSC_VER) && !defined(NOGDI)
#pragma comment( \
    lib, "gdi32")  // Link with gdi32.lib for GetDeviceCaps(). MinGW will require linking with '-lgdi32'
#endif

    float ImGui_ImplWin32_GetDpiScaleForMonitor(void* monitor)
    {
        UINT xdpi = 96, ydpi = 96;
        if (_IsWindows8Point1OrGreater())
        {
            static HINSTANCE shcore_dll = ::LoadLibraryA("shcore.dll");  // Reference counted per-process
            static PFN_GetDpiForMonitor GetDpiForMonitorFn = nullptr;
            if (GetDpiForMonitorFn == nullptr && shcore_dll != nullptr)
                GetDpiForMonitorFn = (PFN_GetDpiForMonitor)::GetProcAddress(shcore_dll, "GetDpiForMonitor");
            if (GetDpiForMonitorFn != nullptr)
            {
                GetDpiForMonitorFn((HMONITOR)monitor, MDT_EFFECTIVE_DPI, &xdpi, &ydpi);
                IM_ASSERT(xdpi == ydpi);  // Please contact me if you hit this assert!
                return xdpi / 96.0f;
            }
        }
#ifndef NOGDI
        const HDC dc = ::GetDC(nullptr);
        xdpi = ::GetDeviceCaps(dc, LOGPIXELSX);
        ydpi = ::GetDeviceCaps(dc, LOGPIXELSY);
        IM_ASSERT(xdpi == ydpi);  // Please contact me if you hit this assert!
        ::ReleaseDC(nullptr, dc);
#endif
        return xdpi / 96.0f;
    }

    float ImGui_ImplWin32_GetDpiScaleForHwnd(void* hwnd)
    {
        HMONITOR monitor = ::MonitorFromWindow((HWND)hwnd, MONITOR_DEFAULTTONEAREST);
        return ImGui_ImplWin32_GetDpiScaleForMonitor(monitor);
    }

    #endif // IS_UWP
        
}  // namespace Internal
}  // namespace HelloImGui


#endif  // #ifdef _WIN32
