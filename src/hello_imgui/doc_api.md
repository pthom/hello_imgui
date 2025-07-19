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


# Run Application while handling the rendering loop
If you want to be in control of the rendering loop, you may use the namespace `HelloImGui::ManualRender` (available since September 2024)

```cpp

namespace ManualRender
{
    // HelloImGui::ManualRender is a namespace that groups functions, allowing fine-grained control over the rendering process:
    // - It is customizable like HelloImGui::Run: initialize it with `RunnerParams` or `SimpleRunnerParams`
    // - `ManualRender::Render()` will render the application for one frame:
    // - Ensure that `ManualRender::Render()` is triggered regularly (e.g., through a loop or other mechanism)
    //   to maintain responsiveness. This method must be called on the main thread.
    //
    // A typical use case is:
    // C++
    //        ```cpp
    //        HelloImGui::RunnerParams runnerParams;
    //        runnerParams.callbacks.ShowGui = ...; // your GUI function
    //        // Optionally, choose between Sleep, EarlyReturn, or Auto for fps idling mode:
    //        // runnerParams.fpsIdling.fpsIdlingMode = HelloImGui::FpsIdlingMode::Sleep; // or EarlyReturn, Auto
    //        HelloImGui::ManualRender::SetupFromRunnerParams(runnerParams);
    //        while (!HelloImGui::GetRunnerParams()->appShallExit)
    //        {
    //            HelloImGui::ManualRender::Render();
    //        }
    //        HelloImGui::ManualRender::TearDown();
    //        ```
    // Python:
    //        ```python
    //        runnerParams = HelloImGui.RunnerParams()
    //        runnerParams.callbacks.show_gui = ... # your GUI function
    //        while not hello_imgui.get_runner_params().app_shall_exit:
    //            hello_imgui.manual_render.render()
    //        hello_imgui.manual_render.tear_down()
    //        ```
    //
    // **Notes:**
    //  1. Depending on the configuration (`runnerParams.fpsIdling.fpsIdlingMode`), `HelloImGui` may enter
    //     an idle state to reduce CPU usage, if no events are received (e.g., no input or interaction).
    //     In this case, `Render()` will either sleep or return immediately.
    //     By default,
    //       - On Emscripten, `ManualRender::Render()` will return immediately to avoid blocking the main thread.
    //       - On other platforms, it will sleep
    //  2. If initialized with `RunnerParams`, a copy of the `RunnerParams` will be made
    //     (which can be accessed with `HelloImGui::GetRunnerParams()`).

    // Initializes the rendering with the full customizable `RunnerParams`.
    // This will initialize the platform backend (SDL, Glfw, etc.) and the rendering backend (OpenGL, Vulkan, etc.).
    // A distinct copy of `RunnerParams` is stored internally.
    void SetupFromRunnerParams(const RunnerParams& runnerParams);

    // Initializes the rendering with `SimpleRunnerParams`.
    // This will initialize the platform backend (SDL, Glfw, etc.) and the rendering backend (OpenGL, Vulkan, etc.).
    void SetupFromSimpleRunnerParams(const SimpleRunnerParams& simpleParams);

    // Initializes the renderer with a simple GUI function and additional parameters.
    // This will initialize the platform backend (SDL, Glfw, etc.) and the rendering backend (OpenGL, Vulkan, etc.).
    void SetupFromGuiFunction(
        const VoidFunction& guiFunction,
        const std::string& windowTitle = "",
        bool windowSizeAuto = false,
        bool windowRestorePreviousGeometry = false,
        const ScreenSize& windowSize = DefaultWindowSize,
        float fpsIdle = 10.f
    );

    // Renders the current frame. Should be called regularly to maintain the application's responsiveness.
    void Render();

    // Tears down the renderer and releases all associated resources.
    // This will release the platform backend (SDL, Glfw, etc.) and the rendering backend (OpenGL, Vulkan, etc.).
    // After calling `TearDown()`, the InitFromXXX can be called with new parameters.
    void TearDown();
} // namespace ManualRender

```

----

# Place widgets in a DPI-aware way


Special care must be taken in order to correctly handle screen with high DPI
 (for example, almost all recent laptops screens).

Using ImVec2 with fixed values is *almost always a bad idea* if you intend your
application to be used on high DPI screens!
Otherwise, widgets might be misplaced or too small on different screens and/or OS.

Instead, you should use scale your widgets and windows relatively to the font size,
as is done with the [em CSS Unit](https://www.w3schools.com/cssref/css_units.php).


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

// __HelloImGui::PixelToEm()__ converts a Vec2 in pixels coord to a Vec2 in em units
ImVec2 PixelsToEm(ImVec2 pixels);

// __HelloImGui::PixelSizeToEm()__ converts a size in pixels coord to a size in em units
float  PixelSizeToEm(float pixelSize);

```

----
# Load fonts
See [hello_imgui_font.h](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/hello_imgui_font.h).
```cpp

    // When loading fonts, use
    //          HelloImGui::LoadFont(..)
    //      or
    //      	HelloImGui::LoadDpiResponsiveFont()
    //
    // Use these functions instead of ImGui::GetIO().Fonts->AddFontFromFileTTF(),
    // because they will automatically adjust the font size to account for HighDPI,
    // and will help you to get consistent font size across different OSes.

    //
    // Font loading parameters: several options are available (color, merging, range, ...)
    struct FontLoadingParams
    {
        // if true, the font size will be adjusted automatically to account for HighDPI
        //
        bool adjustSizeToDpi = true;

        // if true, the font will be merged to the last font
        bool mergeToLastFont = false;

        // if true, the font will be loaded using colors
        // (requires freetype, enabled by IMGUI_ENABLE_FREETYPE)
        bool loadColor = false;

        // if true, the font will be loaded using HelloImGui asset system.
        // Otherwise, it will be loaded from the filesystem
        bool insideAssets = true;

        // ImGui native font config to use
        ImFontConfig fontConfig = ImFontConfig();
    };


    // Loads a font with the specified parameters
    ImFont* LoadFont(
        const std::string & fontFilename, float fontSize,
        const FontLoadingParams & params = {});

    ImFont* LoadFontTTF(
        const std::string & fontFilename,
        float fontSize,
        ImFontConfig config = ImFontConfig()
    );

    ImFont* LoadFontTTF_WithFontAwesomeIcons(
        const std::string & fontFilename,
        float fontSize,
        ImFontConfig configFont = ImFontConfig()
    );

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
// This function can be redirected with setLoadAssetFileDataFunction. If not redirected,
// it calls DefaultLoadAssetFileData.
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
                    const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1,1));

// `HelloImGui::ImageFromAsset(const char *assetPath, size, ...)`:
// will display a static image from the assets, with a colored background and a border.
void ImageFromAssetWithBg(const char *assetPath, const ImVec2& size = ImVec2(0, 0),
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


// `HelloImGui::ImageAndSize HelloImGui::ImageAndSizeFromAsset(assetPath)`:
// will return the texture ID and the size of an image loaded from the assets.
struct ImageAndSize
{
    ImTextureID textureId = ImTextureID(0);
    ImVec2 size = ImVec2(0.f, 0.f);
};
ImageAndSize ImageAndSizeFromAsset(const char *assetPath);


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

// `GetRunnerParams()`:  a convenience function that will return the runnerParams
// of the current application
    RunnerParams* GetRunnerParams();

// `IsUsingHelloImGui()`: returns true if the application is using HelloImGui
    bool IsUsingHelloImGui();

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

// `GetBackendDescription()`: returns a string with the backend info
// Could be for example:
//     "Glfw - OpenGL3"
//     "Glfw - Metal"
//     "Sdl - Vulkan"
std::string GetBackendDescription();

// `ChangeWindowSize(const ScreenSize &windowSize)`: sets the window size
// (useful if you want to change the window size during execution)
void ChangeWindowSize(const ScreenSize &windowSize);


// `UseWindowFullMonitorWorkArea()`: sets the window size to the monitor work area
// (useful if you want to change the window size during execution)
void UseWindowFullMonitorWorkArea();

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

// `AddDockableWindow()`: will add a dockable window to the current layout.
// Will dock the window to the dockspace it belongs to if forceDockspace is true,
// otherwise will dock it to the last space it was docked to (using saved settings)
void AddDockableWindow(const DockableWindow& dockableWindow, bool forceDockspace = false);

// `RemoveDockableWindow()`: will remove a dockable window from the current layout.
// (dockableWindowName is the label of the window, as provided in the DockableWindow struct)
void RemoveDockableWindow(const std::string& dockableWindowName);

```

----

# Ini settings

## Ini settings location

```cpp

// IniFolderType is an enum which describes where is the base path to store
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

    // AbsolutePath: an absolute path
    // (convenient, but not recommended if targeting multiple platforms)
    AbsolutePath,

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

# Additional Widgets

## InputTextResizable

```cpp

    // `InputTextResizable`: displays a resizable text input widget
    //
    // The `InputTextResizable` widget allows you to create a text input field that can be resized by the user.
    // It supports both single-line and multi-line text input.
    // Note: the size of the widget is expressed in em units.
    // **Usage example:**
    //    C++:
    //       ```cpp
    //       // Somewhere in the application state
    //       (static) InputTextData textInput("My text", true, ImVec2(10, 3));
    //       // In the GUI function
    //       bool changed = InputTextResizable("Label", &textInput);
    //       ```
    //    Python:
    //       ```python
    //       # Somewhere in the application state
    //       text_input = hello_imgui.InputTextData("My text", multiline=True, size_em=ImVec2(10, 3))
    //       # In the GUI function
    //       changed = hello_imgui.input_text_resizable("Label", text_input)
    //       ```
    struct InputTextData
    {
        // The text edited in the input field
        std::string Text;

        // An optional hint displayed when the input field is empty
        // (only works for single-line text input)
        std::string Hint;

        // If true, the input field is multi-line
        bool Multiline = false;

        // If true, the input field is resizable
        bool Resizable = true;

        // The size of the input field in em units
        ImVec2 SizeEm = ImVec2(0, 0);

        InputTextData(const std::string& text = "", bool multiline = false, ImVec2 size_em = ImVec2(0, 0)) : Text(text), Multiline(multiline), SizeEm(size_em) {}
    };
    bool InputTextResizable(const char* label, InputTextData* textInput);

    // Serialization for InputTextData
    // -------------------------------
    // to/from dict
    using DictTypeInputTextData = std::map<std::string, std::variant<std::string, bool, float>>;
    DictTypeInputTextData InputTextDataToDict(const InputTextData& data);
    InputTextData InputTextDataFromDict(const DictTypeInputTextData& dict);
    // to/from string
    std::string InputTextDataToString(const InputTextData& data);
    InputTextData InputTextDataFromString(const std::string& str);

```

## WidgetWithResizeHandle

```cpp

    // WidgetWithResizeHandle: adds a resize handle to a widget
    // Example usage with ImPlot:
    //        void gui()
    //        {
    //            static ImVec2 widget_size(200, 200);
    //            auto myWidgetFunction = []()
    //            {
    //                if (ImPlot::BeginPlot("My Plot", widget_size)) {
    //                    ImPlot::PlotLine("My Line", x.data(), y.data(), 1000);
    //                    ImPlot::EndPlot();
    //                }
    //            };
    //            widget_size = widget_with_resize_handle("plot", myWidgetFunction);
    //        }
    ImVec2 WidgetWithResizeHandle(
        const char* id,
        VoidFunction widgetGuiFunction,
        float handleSizeEm = 1.0f,
        std::optional<VoidFunction> onItemResized = std::nullopt,
        std::optional<VoidFunction> onItemHovered = std::nullopt
        );

``` 

---

# Handling screens with high DPI


_Note: This part is relevant only for more advanced usages. If you use `HelloImGui::LoadFont()`,
 and always use `HelloImGui::EmToVec2()` to place widgets, you do not need to worry about DPI handling_

## OS specificities

There are several important things to know about high-DPI handling within Hello ImGui and Dear ImGui:

1. (virtual) screen coordinates vs (physical) pixels
2. DisplayFramebufferScale: Frame buffer size vs window size
3. FontGlobalScale: display-time font scaling factor
4. How to load fonts with the correct size
5. How to get similar window sizes on different OSes/DPI


## Screen coordinates

Screen coordinates are the coordinates you use to place and size windows on the screen.

**Screen coordinates do not always correspond to physical pixels**

- On macOS/iOS retina screens, a screen coordinate corresponds typically
  to 2x2 physical pixels (but this may vary if you change the display scaling)
- On most Linux distributions, whenever there is a high DPI screen
  you can set the display scale. For example if you set the scale to 300%,
  then a screen coordinate will correspond to 3x3 physical pixels
- On Windows, there are two possible situations:
    - If the application is DPI aware, a screen coordinate corresponds to 1x1 physical pixel,
      and you can use the full extent of your screen resolution.
    - If the application is not DPI aware, a screen coordinate may correspond to 2x2 physical pixels
      (if the display scaling is set to 200% for example). However, the rendering of your application
      will be blurry and will not use the full extent of your screen resolution.
    - Notes:
        - Applications created with HelloImGui are DPI aware by default (when using glfw and sdl backends).
        - SDL applications are normally not DPI aware. However, HelloImGui makes them DPI aware.


## DisplayFramebufferScale
`DisplayFramebufferScale` is the ratio between the frame buffer size and the window size.

The frame buffer size is the size of the internal buffer used by the rendering backend.
It might be bigger than the actual window size.
`ImVec2 ImGui::GetIO().DisplayFramebufferScale` is a factor by which the frame buffer size is bigger than the window size.
It is set by the platform backend after it was initialized, and typically reflects the scaling ratio between
physical pixels and screen coordinates.

Under windows, it will always be (1,1). Under macOS / linux, it will reflect the current display scaling.
It will typically be (2,2) on a macOS retina screen.

Notes:
- You cannot change DisplayFramebufferScale manually, it will be reset at each new frame, by asking the platform backend.


## How to load fonts with the correct size

### Using HelloImGui (recommended)

[`HelloImGui::LoadFont()`](https://pthom.github.io/hello_imgui/book/doc_api.html#load-fonts) will load fonts
 with the correct size, taking into account the DPI scaling.

### Using Dear ImGui
`ImGui::GetIO().Fonts->AddFontFromFileTTF()` loads a font with a given size, in *physical pixels*.
KKDYNFONT: TBC...

## Reproducible physical window sizes (in mm or inches)

### Using HelloImGui
Simply specify a window size that corresponds to theoretical 96 PPI screen (inside `RunnerParams.appWindowParams.windowGeometry.size`)

### Using your own code to create the backend window
If you prefer to create the window by yourself, its physical size in millimeters may vary widely,
depending on the OS and the current screen DPI setting.
Typically under Windows, your window may appear to be very small if your screen is high DPI.

To get a similar window size on different OSes/DPI, you should multiply the window size by `HelloImGui::DpiWindowSizeFactor()`.

Note: DpiWindowSizeFactor() is equal to `CurrentScreenPixelPerInch / 96` under windows and linux, and always 1 under macOS.

## Fine tune DPI Handling

See [`HelloImGui::DpiAwareParams`](https://pthom.github.io/hello_imgui/book/doc_params.html#dpi-aware-params)
for more information on how to fine tune DPI handling when using Hello ImGui.
