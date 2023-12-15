#pragma once

#include <array>
#include <stddef.h>
#include <string>


#define ForDim2(dim) for (size_t dim = 0; dim < 2; dim += 1)


namespace HelloImGui
{
    using ScreenPosition = std::array<int, 2>;
    using ScreenSize = std::array<int, 2>;

    // Note: note related to DPI and high resolution screens:
    // ScreenPosition and ScreenSize are in "Screen Coordinates":
    // Screen coordinates *might* differ from real pixel on high dpi screens; but this depends on the OS.
    // - For example, on apple a retina screenpixel size 3456x2052 might be seen as 1728x1026 in screen coordinates
    // - Under windows, ScreenCoordinates correspond to pixels, even on high density screens
    constexpr ScreenPosition DefaultScreenPosition = {0, 0};
    constexpr ScreenSize DefaultWindowSize = {800, 600};

    struct ScreenBounds
    {
        ScreenPosition position = DefaultScreenPosition;
        ScreenSize size = DefaultWindowSize;

        ScreenPosition TopLeftCorner() const{ return position; }
        ScreenPosition BottomRightCorner() const{ return { position[0] + size[0], position[1] + size[1] }; }
        ScreenPosition Center() const{ return { position[0] + size[0] / 2, position[1] + size[1] / 2 }; }

        bool Contains(ScreenPosition pixel) const;
        ScreenPosition WinPositionCentered(ScreenSize windowSize) const;
        int DistanceFromPixel(ScreenPosition point) const;
        ScreenBounds EnsureWindowFitsThisMonitor(ScreenBounds windowBoundsOriginal) const;
        bool operator==(const ScreenBounds& other) const;
    };



} // namespace BackendApi
