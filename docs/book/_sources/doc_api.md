# API

# Run Application

HelloImGui is extremely easy to use: there is **one** main function in the API, with three overloads.


__HelloImGui::Run()__ will run an application with a single call.

Three signatures are provided:

* `HelloImGui::Run(RunnerParams &)`: full signature, the most customizable version.
  Runs an application whose params and Gui are provided by runnerParams.

* `HelloImGui::Run(const SimpleRunnerParams&)`:
  Runs an application, using simpler params.

* `HelloImGui::Run(guiFunction, windowTitle, windowSize, windowSizeAuto=false, restoreLastWindowGeometry=false, fpsIdle=10)`
  Runs an application, by providing the Gui function, the window title, etc.

Although the API is extremely simple, it is highly customizable, and you can set many options by filling
the elements in the `RunnerParams` struct, or in the simpler  `SimpleRunnerParams`.

__HelloImGui::GetRunnerParams()__  will return the runnerParams of the current application.


----

# Place widgets in a DPI-aware way


Special care must be taken in order to correctly handle screen with high DPI
 (for example, almost all recent laptops screens).

Using ImVec2 with fixed values is *almost always a bad idea* if you intend your
application to be used on high DPI screens!
Otherwise, widgets might be misplaced or too small on different screens and/or OS.

Instead you should use scale your widgets and windows relatively to the font size,
as is done with the [em CSS Unit](https://lyty.dev/css/css-unit.html).


```cpp
//  __HelloImGui::EmToVec2()__ returns an ImVec2 that you can use to size
//  or place your widgets in a DPI independent way.
//  Values are in multiples of the font size (i.e. as in the em CSS unit).
ImVec2 EmToVec2(float x, float y);
ImVec2 EmToVec2(ImVec2 v);

// __HelloImGui::EmSize()__ returns the visible font size on the screen.
float EmSize();
// __HelloImGui::EmSize(nbLines)__ returns a size corresponding to nbLines text lines
float EmSize(float nbLines);
```

----
# Load fonts
See [hello_imgui_font.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/hello_imgui_font.h).
```cpp
    //
    // When loading fonts, use HelloImGui::LoadFont(fontFilename, fontSize, fontLoadingParams)
    //
    // Font loading parameters: several options are available (color, merging, range, ...)
    struct FontLoadingParams
    {
        // if true, the font size will be adjusted automatically to account for HighDPI
        bool adjustSizeToDpi = true;

        // if true, the font will be loaded with the full glyph range
        bool useFullGlyphRange = false;
        // if set, fontConfig.GlyphRanges, and
        //   fontConfig.OversampleH / fontConfig.OversampleV will be set to 1
        //   when useFullGlyphRange is true (this is useful to save memory)
        bool reduceMemoryUsageIfFullGlyphRange = true;

        // if true, the font will be merged to the last font
        bool mergeToLastFont = false;

        // if true, the font will be loaded using colors
        // (requires freetype, enabled by IMGUI_ENABLE_FREETYPE)
        bool loadColor = false;

        // if true, the font will be loaded using HelloImGui asset system.
        // Otherwise, it will be loaded from the filesystem
        bool insideAssets = true;

        // the ranges of glyphs to load:
        //    - if empty, the default glyph range will be used
        //    - you can specify several ranges
        //    - intervals bounds are inclusive
        // (will be translated and stored as a static ImWChar* inside fontConfig)
        std::vector<ImWcharPair> glyphRanges = {};

        // ImGui native font config to use
        ImFontConfig fontConfig = ImFontConfig();

        // if true, the font will be loaded and then FontAwesome icons will be merged to it
        // (deprecated, use mergeToLastFont instead, and load in two steps)
        bool mergeFontAwesome = false;
        ImFontConfig fontConfigFontAwesome = ImFontConfig();
    };

    // When loading fonts, use HelloImGui::LoadFont(FontLoadingParams)
    // ===============================================================
    // instead of ImGui::GetIO().Fonts->AddFontFromFileTTF(), because it will
    // automatically adjust the font size to account for HighDPI, and will spare
    // you headaches when trying to get consistent font size across different OSes.
    // see FontLoadingParams and ImFontConfig
    ImFont* LoadFont(const std::string & fontFilename, float fontSize,
                     const FontLoadingParams & params = {});


```

----

# Applications assets
See [hello_imgui_assets.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/hello_imgui_assets.h).

## Load Assets as data buffer

```cpp

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
```

## Get assets path

```cpp

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

```


## Display images from assets
See [image_from_asset.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/image_from_asset.h).
```cpp

//
//Images are loaded when first displayed, and then cached
// (they will be freed just before the application exits).
//
//For example, given this files structure:
//```
//├── CMakeLists.txt
//├── assets/
//│         └── my_image.jpg
//└── my_app.main.cpp
//```
//
//then, you can display "my_image.jpg", using:
//
//    ```cpp
//    HelloImGui::ImageFromAsset("my_image.jpg");
//    ```


// `HelloImGui::ImageFromAsset(const char *assetPath, size, ...)`: 
// will display a static image from the assets.
void ImageFromAsset(const char *assetPath, const ImVec2& size = ImVec2(0, 0),
                    const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1,1),
                    const ImVec4& tint_col = ImVec4(1,1,1,1),
                    const ImVec4& border_col = ImVec4(0,0,0,0));

// `bool HelloImGui::ImageButtonFromAsset(const char *assetPath, size, ...)`:
// will display a button using an image from the assets.
bool ImageButtonFromAsset(const char *assetPath, const ImVec2& size = ImVec2(0, 0),
                          const ImVec2& uv0 = ImVec2(0, 0),  const ImVec2& uv1 = ImVec2(1,1),
                          int frame_padding = -1,
                          const ImVec4& bg_col = ImVec4(0,0,0,0),
                          const ImVec4& tint_col = ImVec4(1,1,1,1));

// `ImTextureID HelloImGui::ImTextureIdFromAsset(assetPath)`:
// will return a texture ID for an image loaded from the assets.
ImTextureID ImTextureIdFromAsset(const char *assetPath);

// `ImVec2 HelloImGui::ImageSizeFromAsset(assetPath)`:
// will return the size of an image loaded from the assets.
ImVec2 ImageSizeFromAsset(const char *assetPath);

// `ImVec2 HelloImGui::ImageProportionalSize(askedSize, imageSize)`:
//  will return the displayed size of an image.
//     - if askedSize.x or askedSize.y is 0, then the corresponding dimension
//       will be computed from the image size, keeping the aspect ratio.
//     - if askedSize.x>0 and askedSize.y> 0, then the image will be scaled to fit
//       exactly the askedSize, thus potentially changing the aspect ratio.
//  Note: this function is used internally by ImageFromAsset and ImageButtonFromAsset,
//        so you don't need to call it directly.
ImVec2 ImageProportionalSize(const ImVec2& askedSize, const ImVec2& imageSize);

```

----

# Utility functions

```cpp


// `FrameRate(durationForMean = 0.5)`: Returns the current FrameRate.
//  May differ from ImGui::GetIO().FrameRate, since one can choose the duration
//  for the calculation of the mean value of the fps
//  Returns the current FrameRate. May differ from ImGui::GetIO().FrameRate,
//  since one can choose the duration for the calculation of the mean value of the fps
//  (Will only lead to accurate values if you call it at each frame)
float FrameRate(float durationForMean = 0.5f);

// `ImGuiTestEngine* GetImGuiTestEngine()`: returns a pointer to the global instance
//  of ImGuiTestEngine that was initialized by HelloImGui
//  (iif ImGui Test Engine is active).
ImGuiTestEngine* GetImGuiTestEngine();
```

----
# Switch between several layouts
See [hello_imgui.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/hello_imgui.h).
    
```cpp

// In advanced cases when several layouts are available, you can switch between layouts.
// See demo inside
//     https://github.com/pthom/hello_imgui/tree/master/src/hello_imgui_demos/hello_imgui_demodocking/hello_imgui_demodocking.main.cpp

// `SwitchLayout(layoutName)`
//  Changes the application current layout. Only used in advanced cases
//  when several layouts are available, i.e. if you filled
//      runnerParams.alternativeDockingLayouts.
void           SwitchLayout(const std::string& layoutName);

// `CurrentLayoutName()`: returns the name of the current layout
std::string    CurrentLayoutName();
```

----

# Ini settings

## Ini settings location

```cpp

// IniFolderType is an enum which describle where is the base path to store
// the ini file for the application settings.
//
// You can use IniFolderLocation(iniFolderType) to get the corresponding path.
//
// RunnerParams contains the following members, which are used to compute
// the ini file location:
//     iniFolderType           (IniFolderType::CurrentFolder by default)
//     iniFilename             (empty string by default)
//     iniFilename_useAppWindowTitle
//         (true by default: iniFilename is derived from
//          appWindowParams.windowTitle)
//
// iniFilename may contain a subfolder
// (which will be created inside the iniFolderType folder if needed)
//
enum class IniFolderType
{
    // CurrentFolder: the folder where the application is executed
    // (convenient for development, but not recommended for production)
    CurrentFolder,

    // AppUserConfigFolder:
    //      AppData under Windows (Example: C:\Users\[Username]\AppData\Roaming under windows)
    //      ~/.config under Linux
    //      "~/Library/Application Support" under macOS
    // (recommended for production, if settings do not need to be easily accessible by the user)
    AppUserConfigFolder,

    // AppExecutableFolder: the folder where the application executable is located
    // (this may be different from CurrentFolder if the application is launched from a shortcut)
    // (convenient for development, but not recommended for production)
    AppExecutableFolder,

    // HomeFolder: the user home folder
    // (recommended for production, if settings need to be easily accessible by the user)
    HomeFolder,

    // DocumentsFolder: the user documents folder
    DocumentsFolder,

    // TempFolder: the system temp folder
    TempFolder
};

// Returns the path corresponding to the given IniFolderType
std::string IniFolderLocation(IniFolderType iniFolderType);

```

```cpp

// IniSettingsLocation returns the path to the ini file for the application settings.
std::string IniSettingsLocation(const RunnerParams& runnerParams);

// HasIniSettings returns true if the ini file for the application settings exists.
bool HasIniSettings(const RunnerParams& runnerParams);

// DeleteIniSettings deletes the ini file for the application settings.
void DeleteIniSettings(const RunnerParams& runnerParams);

```

----

## Store user settings in the ini file
See [hello_imgui.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/hello_imgui.h).

```cpp

// You may store additional user settings in the application settings.
// This is provided as a convenience only, and it is not intended to store large
// quantities of text data. Use sparingly.

// `SaveUserPref(string userPrefName, string userPrefContent)`:
//  Shall be called in the callback runnerParams.callbacks.BeforeExit
void        SaveUserPref(const std::string& userPrefName, const std::string& userPrefContent);

// `string LoadUserPref(string& userPrefName)`
//  Shall be called in the callback runnerParams.callbacks.PostInit
std::string LoadUserPref(const std::string& userPrefName);
```

----

# Customize Hello ImGui Menus


Hello ImGui provides a default menu and status bar, which you can customize by using the params:
        `RunnerParams.imGuiWindowParams.` `showMenuBar` / `showMenu_App` / `showMenu_View`

If you want to fully customize the menu:
* set `showMenuBar` to true, then set `showMenu_App` and `showMenu_View` params to false
* implement the callback `RunnerParams.callbacks.ShowMenus`:
  it can optionally call `ShowViewMenu` and `ShowAppMenu` (see below).


```cpp

// `ShowViewMenu(RunnerParams & runnerParams)`:
// shows the View menu (where you can select the layout and docked windows visibility
void ShowViewMenu(RunnerParams & runnerParams);

// `ShowAppMenu(RunnerParams & runnerParams)`:
// shows the default App menu (including the Quit item)
void ShowAppMenu(RunnerParams & runnerParams);
```

---

# Handling screens with high DPI

_Note: This part is relevant only for more advanced usages. If you use `HelloImGui::LoadFont()`, and always use `HelloImGui::EmToVec2()` to place widgets, you do not need to worry about DPI handling_ 

## Details on DPI handling on different OS

Let's consider screen whose physical pixel resolution is 3600x2000, but which will displayed with a scaling factor of 200%, so that widgets do not look too small on it.

The way it is handled depends on the OS:
- On MacOS, the screen will be seen as having a resolution of 1800x1000, and the OS handles the resizing by itself.
- On Linux, and on Windows if the application is DPI aware, the screen will be seen as having a resolution of 3600x2000.
- On Windows if the application is not DPI aware, the screen will be seen as having a resolution of 1800x1000

By default, if using the glfw backend, applications will be Dpi aware under windows.
Sdl applications are normally not Dpi aware. However HelloImGui makes them Dpi aware when using the sdl backend.


## Dpi aware Font scaling

`HelloImGui::LoadFont()` will load fonts with the correct size, taking into account the DPI scaling.

If you prefer to use `ImGui::GetIO().Fonts->AddFontFromFileTTF()`, there are two things to know:

1. You should adjust `ImGui::GetIO().FontGlobalScale`:

Under windows and linux, it should be is 1: no rescaling should be done by ImGui.
Under macOS and emscripten, it may need to bet set to 0.5 (for example it will be 0.5 if the dpi scaling is 200%
on a macOS retina screen)

`HelloImGui::ImGuiDefaultFontGlobalScale()` returns the default value that should be stored inside `ImGui::GetIO().FontGlobalScale`.


2. You should adjust the font size when loading a font:

`HelloImGui::DpiFontLoadingFactor()` returns a factor by which you shall multiply your font sizes when loading them.

`HelloImGui::DpiFontLoadingFactor()` corresponds to:
`DpiWindowSizeFactor() * 1.f / ImGui::GetIO().FontGlobalScale`

where DpiWindowSizeFactor() is equal to `CurrentScreenPixelPerInch / 96`
under windows and linux, 1 under macOS
