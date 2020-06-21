#include "hello_imgui/hello_imgui_assets.h"

#ifdef IOS
#include "hello_imgui/internal/platform/getAppleBundleResourcePath.h"
#endif

namespace HelloImGui
{

/// Access font files in application bundle or assets/fonts/
std::string assetFileFullPath(const std::string& assetFilename)
{
#if defined(IOS)
    std::string path = getAppleBundleResourcePath(assetFilename.c_str());
        return path;
#elif defined(__EMSCRIPTEN__)
    std::string path = std::string("/") + assetFilename;
        return path;
#else
    std::string path = std::string(HELLOIMGUI_ASSETSDIR) + "/" + assetFilename;
    return path;
#endif
}
}