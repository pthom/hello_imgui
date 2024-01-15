#include "hello_imgui/screen_bounds.h"

#include <sstream>
#include <vector>
#include <string>

namespace HelloImGui
{

bool ScreenBounds::Contains(ScreenPosition pixel) const{
    ForDim2(dim) {
        if (pixel[dim] < TopLeftCorner()[dim])
            return false;
        if (pixel[dim] >= BottomRightCorner()[dim])
            return false;
    }
    return true;
}

ScreenPosition ScreenBounds::WinPositionCentered(ScreenSize windowSize) const {
    return {
        Center()[0] - windowSize[0] / 2,
        Center()[1] - windowSize[1] / 2
    };
}

int ScreenBounds::DistanceFromPixel(ScreenPosition point) const
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

ScreenBounds ScreenBounds::EnsureWindowFitsThisMonitor(ScreenBounds windowBoundsOriginal) const
{
    auto &self = *this;
    ScreenBounds windowBoundsNew = windowBoundsOriginal;
    ForDim2(dim)
    {
        // 1. if window is to the left or to the top, move it
        if (windowBoundsNew.position[dim] < self.position[dim])
            windowBoundsNew.position[dim] = self.position[dim];
        // 2.1 if the window is too big and does not fit the bottom right corner, try to move it
        if (windowBoundsNew.BottomRightCorner()[dim] >= self.BottomRightCorner()[dim])
            windowBoundsNew.position[dim] = self.BottomRightCorner()[dim] - 1 - windowBoundsNew.size[dim];
        // Redo 1. if window is to the left or to the top, move it (since we may have moved)
        if (windowBoundsNew.position[dim] < self.position[dim])
            windowBoundsNew.position[dim] = self.position[dim];
        // 3. if it was not enough, resize it
        if (windowBoundsNew.BottomRightCorner()[dim] >= self.BottomRightCorner()[dim])
            windowBoundsNew.size[dim] = self.size[dim];
    }
    return windowBoundsNew;
}

bool ScreenBounds::operator==(const ScreenBounds& other) const
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



namespace details
{
    std::vector<std::string> splitString(const std::string &s, char delimiter)
    {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter))
        {
            tokens.push_back(token);
        }
        return tokens;
    }
}

std::string IntPairToString(std::array<int, 2> v)
{
    std::stringstream ss;
    ss << v[0] << "," << v[1];
    return ss.str();
}

std::array<int, 2> StringToIntPair(const std::string& s)
{
    auto items = details::splitString(s, ',');
    if (items.size() != 2)
        return {-1, -1};

    std::array<int, 2> r;
    for (size_t i = 0; i < 2; ++i)
    {
        try
        {
            int asInt = std::stoi(items[i]);
            r[i] = asInt;
        }
        catch(const std::exception&)
        {
            return {-1, -1};
        }
    }
    return r;
}

} // namespace BackendApi
