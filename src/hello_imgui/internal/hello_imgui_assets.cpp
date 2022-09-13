#include "hello_imgui/hello_imgui_assets.h"

#if defined(IOS)
#include "hello_imgui/internal/platform/getAppleBundleResourcePath.h"
#else
#include "hello_imgui/internal/whereami/whereami_cpp.h"
#endif

#ifdef _MSC_VER
#include <sys/types.h>
#include <sys/stat.h>
static bool directoryExist(const std::string& dir)
{
  struct stat st;
  if (stat(dir.c_str(), &st) == -1)
    return false;
  bool isDir = ((st.st_mode & S_IFMT) == S_IFDIR);
  return isDir;
}
#endif

#ifdef HELLOIMGUI_USE_SDL_OPENGL3
#include "SDL_system.h"
#endif

#include "hello_imgui/hello_imgui_error.h"
#include <fstream>
#include <sstream>


namespace HelloImGui
{
  std::string gAssetsSubfolderFolderName = "assets";

  std::string gOverrideAssetsFolder = "";

/// Access font files in application bundle or assets/fonts/
std::string assetFileFullPath(const std::string& assetFilename)
{
    if (!gOverrideAssetsFolder.empty())
    {
        std::string path = gOverrideAssetsFolder + "/" + assetFilename;
        return path;
    }
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
#elif defined(_MSC_VER)
  // For msvc, we have to deal with the fact that the exe could be in a subfolder "Debug" or "Release" of ${CMAKE_CURRENT_BINARY_DIR}
  // whereas the assets folder is located directly inside ${CMAKE_CURRENT_BINARY_DIR}
  std::string assetsFolder = wai_getExecutableFolder_string() + "/" + gAssetsSubfolderFolderName;
  if (!directoryExist(assetsFolder))
  {
      assetsFolder = wai_getExecutableFolder_string() + "/../" + gAssetsSubfolderFolderName;
      if (!directoryExist(assetsFolder))
      {
          HIMG_THROW_STRING(std::string("Cannot find assets folder"));
      }
  }
  std::string path = assetsFolder + "/" + assetFilename;
  return path;
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

    r.dataSize = (std::size_t)ifs.tellg();
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
    std::string fullPath = assetFileFullPath(assetPath);
    if (!r.data)
        r = LoadAssetFileData_Impl(fullPath.c_str());
    if (!r.data)
    {
        std::stringstream msg;
        msg << "LoadAssetFileData: cannot load " << assetPath << " (also tried " << fullPath << ")\n";
        msg << "(you can call HelloImGui::setAssetsFolder() to change the assets default location.";
        HIMG_THROW_STRING(msg.str());
    }
    return r;
}

void FreeAssetFileData(AssetFileData * assetFileData)
{
    free(assetFileData->data);
    assetFileData = nullptr;
}

void overrideAssetsFolder(const char* folder)
{
    gOverrideAssetsFolder = folder;
}

void setAssetsFolder(const char* folder)
{
    gOverrideAssetsFolder = folder;
}

#endif // #ifdef HELLOIMGUI_USE_SDL_OPENGL3


}  // namespace HelloImGui