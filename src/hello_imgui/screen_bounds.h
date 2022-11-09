#pragma once

#include <array>
#include <stddef.h>


namespace HelloImGui
{
    using ScreenPosition = std::array<int, 2>;
    using ScreenSize = std::array<int, 2>;

#define ForDim2(dim) for (size_t dim = 0; dim < 2; dim += 1)

    struct ScreenBounds
    {
        ScreenPosition position = {0, 0};
        ScreenSize size = {100, 100};

        ScreenPosition TopLeftCorner() const{ return position; }
        ScreenPosition BottomRightCorner() const{ return { position[0] + size[0], position[1] + size[1] }; }
        ScreenPosition Center() const{ return { position[0] + size[0] / 2, position[1] + size[1] / 2 }; }

        bool Contains(ScreenPosition pixel) const{
            ForDim2(dim) {
                if (pixel[dim] < TopLeftCorner()[dim])
                    return false;
                if (pixel[dim] >= BottomRightCorner()[dim])
                    return false;
            }
            return true;
        }

        ScreenPosition WinPositionCentered(ScreenSize windowSize) const {
            return {
                Center()[0] - windowSize[0] / 2,
                Center()[1] - windowSize[1] / 2
            };
        }

        int DistanceFromPixel(ScreenPosition point) const
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
            ForDim2(dim)
                distance += distFromInterval(TopLeftCorner()[dim], BottomRightCorner()[dim], point[dim]);
            return distance;
        }

        ScreenBounds EnsureWindowFitsThisMonitor(ScreenBounds windowBoundsOriginal) const
        {
            auto &self = *this;
            ScreenBounds windowBoundsNew = windowBoundsOriginal;
            ForDim2(dim)
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

        bool operator==(const ScreenBounds& other) const
        {
            auto &self = *this;
            ForDim2(dim)
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
