#include "main_screen_resolution.h"

ImVec2 MainScreenResolution_Vec2()
{
    auto res = MainScreenResolution();
    return { (float)res[0], (float)res[1] };
}

#if defined(_WIN32)
#undef APIENTRY
#include <windows.h>
std::array<int, 2> MainScreenResolution()
{
  return {
    GetSystemMetrics(SM_CXSCREEN),
    GetSystemMetrics(SM_CYSCREEN)
  };
}
#elif defined(__ANDROID__)
std::array<int, 2> MainScreenResolution()
{
  return { 1024, 768 };
}
#elif defined(__linux__)
#include <X11/Xlib.h>
std::array<int, 2> MainScreenResolution()
{
  Display* d = XOpenDisplay(NULL);
  Screen* s  = DefaultScreenOfDisplay(d);
  return { s->width, s->height };
}
#else
std::array<int, 2> MainScreenResolution()
{
    return { 1280, 720 };
}
#endif
