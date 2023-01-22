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


/**
@@md#LoadAssetFileData

* `AssetFileData LoadAssetFileData(const char *assetPath)` will load an entire asset file into memory.
 This works on all platforms, including android.
 ```cpp
    struct AssetFileData
    {
        void * data = nullptr;
        size_t dataSize = 0;
    };
 ```
* `FreeAssetFileData(AssetFileData * assetFileData)` will free the memory.

  Note about ImGui: "ImGui::GetIO().Fonts->AddFontFromMemoryTTF" takes ownership of the data
  and will free the memory for you.

@@md
**/
struct AssetFileData
{
    void * data = nullptr;
    size_t dataSize = 0;
};

AssetFileData LoadAssetFileData(const char *assetPath);
void FreeAssetFileData(AssetFileData * assetFileData);



/**
@@md#assetFileFullPath

`std::string AssetFileFullPath(const std::string& assetRelativeFilename)` will return the path to assets.

This works under all platforms __except Android__.
For compatibility with Android and other platforms, prefer to use `LoadAssetFileData` whenever possible.

* Under iOS it will give a path in the app bundle (/private/XXX/....)
* Under emscripten, it will be stored in the virtual filesystem at "/"
* Under Android, assetFileFullPath is *not* implemented, and will throw an error:
  assets can be compressed under android, and you cannot use standard file operations!
  Use LoadAssetFileData instead

@@md
*/
std::string AssetFileFullPath(const std::string& assetRelativeFilename);
inline std::string assetFileFullPath(const std::string& assetRelativeFilename) { return AssetFileFullPath(assetRelativeFilename); }

// Returns true if this asset file exists
bool AssetExists(const std::string& assetRelativeFilename);

extern std::string gAssetsSubfolderFolderName;  // "assets" by default

// Sets the assets folder location
// (when using this, automatic assets installation on mobile platforms may not work)
void SetAssetsFolder(const char* folder);
void SetAssetsFolder(const std::string& folder);
void overrideAssetsFolder(const char* folder); // synonym

} // namespace HelloImGui
