#pragma once

#include <array>

namespace BackendApi
{
    using ScreenPosition = std::array<int, 2>;
    using ScreenSize = std::array<int, 2>;


    constexpr int WindowPositionUnspecified = -1000000;
    constexpr int WindowPositionCentered = -3000000;

    constexpr std::array<int, 2> Range2 = {0, 1};

    struct ScreenBounds
    {
        ScreenPosition position = {WindowPositionUnspecified, WindowPositionUnspecified};
        ScreenSize size = {100, 100};

        ScreenPosition TopLeftCorner() { return position; }
        ScreenPosition BottomRightCorner() { return { position[0] + size[0], position[1] + size[1] }; }
        ScreenPosition Center() { return { position[0] + size[0] / 2, position[1] + size[1] / 2 }; }

        bool Contains(ScreenPosition pixel) {
            for (int dim : Range2) {
                if (pixel[dim] < TopLeftCorner()[dim])
                    return false;
                if (pixel[dim] >= BottomRightCorner()[dim])
                    return false;
            }
            return true;
        }

        ScreenPosition WinPositionCentered(ScreenSize windowSize) {
            return {
                Center()[0] - windowSize[0] / 2,
                Center()[1] - windowSize[1] / 2
            };
        }

        int DistanceFromPixel(ScreenPosition point)
        {
            auto distFromInterval = [](int a, int b, int x) {
                if (x < a)
                    return a - x;
                else if (x > b)
                    return x - b;
                else
                    return 0;
            };
            int distance = 0;
            for (int dim : Range2)
                distance += distFromInterval(TopLeftCorner()[dim], BottomRightCorner()[dim], point[dim]);
            return distance;
        }

        ScreenBounds EnsureWindowFitsThisMonitor(ScreenBounds windowBoundsOriginal)
        {
            auto &self = *this;
            ScreenBounds windowBoundsNew = windowBoundsOriginal;
            for (int dim: Range2)
            {
                // if window is to the left or to the top, move it
                if (windowBoundsNew.position[dim] < self.position[dim])
                    windowBoundsNew.position[dim] = self.position[dim];
                // if the window is too big and does not fit the bottom right corner, try to move it
                if (windowBoundsNew.BottomRightCorner()[dim] >= self.BottomRightCorner()[dim])
                    windowBoundsNew.position[dim] = self.BottomRightCorner()[dim] - 1 - windowBoundsNew.size[dim];
                // if it was not enough, resize it
                if (windowBoundsNew.BottomRightCorner()[dim] >= self.BottomRightCorner()[dim])
                    windowBoundsNew.size[dim] = self.size[dim];
            }
            return windowBoundsNew;
        }

        bool operator==(const ScreenBounds& other)
        {
            auto &self = *this;
            for (int dim: Range2)
            {
                if (self.size[dim] != other.size[dim])
                    return false;
                if (self.position[dim] != other.position[dim])
                    return false;
            }
            return true;
        }
    };
} // namespace BackendApi
