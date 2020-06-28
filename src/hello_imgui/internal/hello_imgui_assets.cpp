#include "hello_imgui/hello_imgui_assets.h"

#if defined(IOS)
#include "hello_imgui/internal/platform/getAppleBundleResourcePath.h"
#else
#include "hello_imgui/internal/whereami/whereami_cpp.h"
#endif

#ifdef HELLOIMGUI_USE_SDL_OPENGL3
#include "SDL_system.h"
#endif

#include "hello_imgui/hello_imgui_error.h"
#include <fstream>

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
#elif defined(__ANDROID__)
    // Under android, assets can be compressed
    // You cannot use standard file operations!`
    (void)assetFilename;
    HIMG_THROW("assetFileFullPath does not work on android!");
#else
    std::string assetsFolder = wai_getExecutableFolder_string() + "/assets/";
    std::string path = assetsFolder + assetFilename;
    return path;
#endif
}



#ifdef HELLOIMGUI_USE_SDL_OPENGL3

AssetFileData LoadAssetFileData(const char *assetPath)
{
    AssetFileData r;
    r.data = SDL_LoadFile(assetPath, &r.dataSize);
    #ifndef __ANDROID__
        if (!r.data)
        {
            std::string otherPath = assetFileFullPath(assetPath);
            r.data = SDL_LoadFile(otherPath.c_str(), &r.dataSize);
        }
    #endif
    #ifdef _MSC_VER
        if (!r.data)
        {
            std::string otherPath2 =
            wai_getExecutableFolder_string() + "/../assets/" + assetPath;
            r.data = SDL_LoadFile(otherPath2.c_str(), &r.dataSize);
        }
#endif
    if (!r.data)
        HIMG_THROW_STRING(std::string("LoadAssetFileData: cannot load ") + assetPath);
    return r;
}

void FreeAssetFileData(AssetFileData * assetFileData)
{
    SDL_free(assetFileData->data);
    assetFileData = nullptr;
}


#else

AssetFileData LoadAssetFileData_Impl(const char *assetPath)
{
    AssetFileData r;

    std::ifstream ifs(assetPath, std::ios::binary | std::ios::ate);
    if (!ifs.good())
        return AssetFileData();

    r.dataSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    r.data = malloc(r.dataSize);

    if (ifs.read((char *)r.data, r.dataSize))
        return r;
    else
    {
        free(r.data);
        return AssetFileData();
    }
}

AssetFileData LoadAssetFileData(const char *assetPath)
{
    AssetFileData r = LoadAssetFileData_Impl(assetPath);
    if (!r.data)
    {
        std::string otherPath = assetFileFullPath(assetPath);
        r = LoadAssetFileData_Impl(otherPath.c_str());
    }
    if (!r.data)
        HIMG_THROW_STRING(std::string("LoadAssetFileData: cannot load ") + assetPath);
    return r;
}

void FreeAssetFileData(AssetFileData * assetFileData)
{
    free(assetFileData->data);
    assetFileData = nullptr;
}


#endif // #ifdef HELLOIMGUI_USE_SDL_OPENGL3


}  // namespace HelloImGui