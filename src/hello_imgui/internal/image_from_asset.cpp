#include "image_from_asset.h"

namespace HelloImGui
{
    ImVec2 ImageProportionalSize(const ImVec2& askedSize, const ImVec2& imageSize)
    {
        ImVec2 r(askedSize);

        if ((r.x == 0.f) && (r.y == 0.f))
            r = imageSize;
        else if (r.y == 0.f)
            r.y = imageSize.y / imageSize.x * r.x;
        else if (r.x == 0.f)
            r.x = imageSize.x / imageSize.y * r.y;
        return r;
    }
}
