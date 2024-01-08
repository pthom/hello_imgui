#pragma once
#include <string>

namespace HelloImGui
{
/**
@@md#AssetsStructure

Assets located beside the application CMakeLists are embedded automatically.

For example, you can have the following project structure:
```
my_app/
├── CMakeLists.txt        # Your app's CMakeLists
├── assets/               # Its assets: for mobile devices and emscripten
│         └── fonts/            # they are embedded automatically by hello_imgui_add_app.cmake
│             └── my_font.ttf
├── my_app.main.cpp       # Its source code
```

Then you can load the asset "fonts/my_font.ttf", on all platforms.

@@md
*/


// @@md#LoadAssetFileData

struct AssetFileData
{
    void * data = nullptr;
    size_t dataSize = 0;
};

// LoadAssetFileData(const char *assetPath)`
// Will load an entire asset file into memory. This works on all platforms,
// including android.
// You *have* to call FreeAssetFileData to free the memory, except if you use
// ImGui::GetIO().Fonts->AddFontFromMemoryTTF, which will take ownership of the
// data and free it for you.
AssetFileData LoadAssetFileData(const char *assetPath);

// FreeAssetFileData(AssetFileData *)
// Will free the memory.
// Note: "ImGui::GetIO().Fonts->AddFontFromMemoryTTF" takes ownership of the data
// and will free the memory for you.
void FreeAssetFileData(AssetFileData * assetFileData);
// @@md


// @@md#assetFileFullPath

//`std::string AssetFileFullPath(const std::string& assetRelativeFilename)`
// will return the path to assets.
//
// This works under all platforms *except Android*
// For compatibility with Android and other platforms, prefer to use `LoadAssetFileData`
// whenever possible.
//    * Under iOS it will give a path in the app bundle (/private/XXX/....)
//    * Under emscripten, it will be stored in the virtual filesystem at "/"
//    * Under Android, assetFileFullPath is *not* implemented, and will throw an error:
//      assets can be compressed under android, and you can't use standard file operations!
//      Use LoadAssetFileData instead
std::string AssetFileFullPath(const std::string& assetRelativeFilename,
                              bool assertIfNotFound = true);

// Returns true if this asset file exists
bool AssetExists(const std::string& assetRelativeFilename);

// Sets the assets folder location
// (when using this, automatic assets installation on mobile platforms may not work)
void SetAssetsFolder(const std::string& folder);

// @@md



// Legacy API, kept for compatibility
void SetAssetsFolder(const char* folder);
inline std::string assetFileFullPath(const std::string& assetRelativeFilename, bool assertIfNotFound = true)
    { return AssetFileFullPath(assetRelativeFilename, assertIfNotFound); }
void overrideAssetsFolder(const char* folder); // synonym of SetAssetsFolder

extern std::string gAssetsSubfolderFolderName;  // "assets" by default

} // namespace HelloImGui
