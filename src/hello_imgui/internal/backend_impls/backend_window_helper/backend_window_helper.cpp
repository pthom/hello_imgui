#include "backend_window_helper.h"

#include <cstdio>


namespace HelloImGui { namespace BackendApi
{
    std::vector<ScreenBounds> WorkAreasOrDefault(std::vector<ScreenBounds> workAreas)
    {
        if (!workAreas.empty())
            return workAreas;
        // The app then starts: its window is placed on this area, and renders (hidden, or shown when the display wakes)
        static bool warned = false;
        if (!warned)
        {
            fprintf(stderr, "HelloImGui: no monitor found (on macOS, the display may be asleep): "
                            "using a 1920x1080 work area\n");
            warned = true;
        }
        return { ScreenBounds{{0, 0}, {1920, 1080}} };
    }
}}
