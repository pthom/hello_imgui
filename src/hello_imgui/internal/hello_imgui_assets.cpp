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
#include <sstream>
#include <vector>
#include <stdio.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

namespace FileUtils
{
    bool IsRegularFile(const std::string& filename)
    {
        FILE *f = fopen(filename.c_str(), "r");
        bool found = (f != NULL);
        if (f)
            fclose(f);
        return found;
    }

    std::string GetCurrentDirectory()
    {
        char buffer[2000];
#ifdef _WIN32
        _getcwd(buffer, 2000);
#else
        char *b = getcwd(buffer, 2000);
        (void)b;
#endif
        std::string r = buffer;
        return r;
    }
}


namespace HelloImGui
{
std::string gAssetsSubfolderFolderName = "assets";

std::string gOverrideAssetsFolder = "";

void overrideAssetsFolder(const char* folder)
{
    gOverrideAssetsFolder = folder;
}

void setAssetsFolder(const char* folder)
{
    gOverrideAssetsFolder = folder;
}

struct AssetFolderWithDesignation
{
    std::string folder;
    std::string designation;
};

std::vector<AssetFolderWithDesignation> computePossibleAssetsFolders()
{
    std::vector<AssetFolderWithDesignation> r;

    // 1. Search inside gOverrideAssetsFolder set by setAssetsFolder()
    if (! gOverrideAssetsFolder.empty())
        r.push_back({gOverrideAssetsFolder, "folder provided by HelloImGui::setAssetsFolder()"});

    // 2. Search inside a subfolder of the exe
    #if !defined(HELLOIMGUI_MOBILEDEVICE) && !defined(__EMSCRIPTEN__)
    {
        r.push_back({wai_getExecutableFolder_string() + "/" + gAssetsSubfolderFolderName, "exe_folder/assets"});
        #ifdef _MSC_VER
        {
            // For msvc, we have to deal with the fact that the exe could be in a subfolder "Debug" or "Release" of ${CMAKE_CURRENT_BINARY_DIR}
            // whereas the assets folder is located directly inside ${CMAKE_CURRENT_BINARY_DIR}
            r.push_back({wai_getExecutableFolder_string() + "/../" + gAssetsSubfolderFolderName, "exe_folder/../assets"});
        }
        #endif
    }
    #endif

    // 3. Search inside a subfolder of the current working directory
    #if !defined(HELLOIMGUI_MOBILEDEVICE)
    {
        r.push_back({FileUtils::GetCurrentDirectory() +  "/" + gAssetsSubfolderFolderName, "current_folder/assets"});
    }
    #endif

    // 3. For emscripten, search at "/"
    #ifdef __EMSCRIPTEN__
    {
        r.push_back({"/", "root folder for emscripten"});
        r.push_back({"/" + gAssetsSubfolderFolderName , "root assets folder for emscripten"});
    }
    #endif

    return r;
}

/// Access font files in application bundle or assets/fonts/
std::string assetFileFullPath(const std::string& assetFilename)
{
#if defined(IOS)
    std::string path = getAppleBundleResourcePath(assetFilename.c_str());
    if (! FileUtils::IsRegularFile(path))
        return  "";
    return path;
#elif defined(__ANDROID__)
    // Under android, assets can be compressed
    // You cannot use standard file operations!`
    (void)assetFilename;
    HIMG_THROW("assetFileFullPath does not work on android!");
#else
    auto possibleAssetsFolders = computePossibleAssetsFolders();
    for (auto assetsFolder: possibleAssetsFolders)
    {
        std::string path = assetsFolder.folder + "/" + assetFilename;
        if (FileUtils::IsRegularFile(path))
            return path;
    }
    // Display nice message on error
    {
        std::string errorMessage;
        errorMessage += "assetFileFullPath(" + assetFilename + ") failed!\n";
        errorMessage += "    Tried the following assets folders:\n";
        for (auto assetsFolder: possibleAssetsFolders)
        {
            errorMessage += "        " + assetsFolder.designation + ":\n";
            errorMessage += "            " + assetsFolder.folder + "\n";
        }
        errorMessage += "    (you can call HelloImGui::setAssetsFolder() to set the default search location)\n";
        HIMG_THROW_STRING(errorMessage);
    }
#endif
}


#ifdef HELLOIMGUI_USE_SDL_OPENGL3

AssetFileData LoadAssetFileData(const char *assetPath)
{
    #ifdef __ANDROID__
    {
        AssetFileData r;
        testedPaths.push_back(assetPath);
        r.data = SDL_LoadFile(assetPath, &r.dataSize);
        if (!r.data)
            HIMG_THROW_STRING(std::string("LoadAssetFileData: cannot load ") + assetPath);

        return r;
    }
    #else
    {
        std::string assetFullPath = assetFileFullPath(assetPath);

        AssetFileData r;
        r.data = SDL_LoadFile(assetFullPath.c_str(), &r.dataSize);
        if (! r.data)
        {
            std::string errorMessage = std::string("LoadAssetFileData(") + assetPath + ")\n";
            errorMessage += "==> SDL_LoadFile(" + assetFullPath + ") failed !!!!\n";
            HIMG_THROW_STRING(errorMessage);
        }

        return r;
    }
    #endif
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
    std::string fullPath = assetFileFullPath(assetPath);
    AssetFileData r = LoadAssetFileData_Impl(fullPath.c_str());
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

#endif // #ifdef HELLOIMGUI_USE_SDL_OPENGL3


}  // namespace HelloImGui