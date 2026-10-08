*Version numbers are synced between Hello ImGui and Dear ImGui Bundle, using the scheme `major.minor.patch` where `patch = ImGui_patch × 100 + release`. For example, ImGui v1.92.6 → v1.92.600, and a bugfix release becomes v1.92.601.*

# Changes in upcoming release

**Touch screens: scroll with a swipe**
* A finger that drags the content of a window scrolls it, with inertia after the release and a bounce at the end (the content pulled past its end follows the finger with a growing resistance, an inertia that reaches the end overshoots, both spring back), as on a phone, even when the drag starts on a button: a tap still clicks it (when the finger lifts), two taps are a double click, and a short hold then a drag goes to the widget (a slider, a text selection; a ring around the finger shows when the hold took effect). `RunnerParams::touchScrollMode` (`Auto`: when the input is a touch screen; `Always`: also with the mouse, to try it on a desktop; `Disabled`). Demo: `hello_touch_scroll`.
* A finger still for half a second, then lifted, is a right click (the context menus), as Windows' press-and-hold: `RunnerParams::touchLongPressIsRightClick`. A ring around the finger shows when the lift will right click; a move after it cancels the right click, and the widget under the finger keeps its press (a slider dragged after a pause, a text selection that grows). Not on a widget that acted on the press already (a button that repeats while held).
* `HelloImGui::SetItemTakesTouchDrags()`, called right after a widget that is dragged (a plot, a node editor, a canvas): a press on it goes to the widget at once, without the hold, even in a window that scrolls (a swipe that starts on it then does not scroll the window). A drag along an axis the window does not scroll already went to the widget. `SetItemTakesTouchDrags(false)`: a finger held still then lifted on the widget is no right click (a piano key).
* In the browser, `HelloImGui::SetTapOpensUrl(rectMin, rectMax, url)`: a tap on a rectangle opens a url in a new tab, from the touch itself (a browser allows a new tab only there, and ImGui sees a tap two frames later). Several rectangles per frame: the links of a text.
* In the browser, the virtual keyboard of a phone: a text widget that is active shows a keyboard button next to it; a tap on it, or on the widget again, opens the keyboard, and what is typed goes to the widget (the browser shows a keyboard only for a text field focused inside a touch handler, which a frame cannot do). The keyboard stays open when the application activates the widget again (Enter, then `SetKeyboardFocusHere()`: a find bar, a console). Once the user hid the keyboard, the button stays hidden until another widget becomes active.
* In the browser, two fingers that pinch scale the font (`style.FontScaleMain`): `RunnerParams::touchPinchMode` (`FontScale`, `Disabled`), and `touchPinchInterruptsWidgets` (whether the second finger takes the press from a widget that holds it). ImGui also learns whether the pointer is a mouse, a finger or a pen (with GLFW, which has no API for it), and keeps no pointer between two touches.

**Mouse wheel:** the wheel stays with the window it scrolls, as in a browser. A page scrolled with the wheel keeps scrolling when a plot or an image passes under the mouse, and the plot does not zoom; a wheel that starts on the plot zooms it. The page lets go of the wheel when the mouse moves, or 0.7 s after the last wheel event.
* A widget of your own that reads the wheel claims it while hovered, as ImPlot and ImmVision do: `if (ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY)) zoom *= powf(1.1f, ImGui::GetIO().MouseWheel);`. Without the claim, in a page that scrolls, the widget never sees the wheel.
* `RunnerParams::wheelSession` (default true): false gives Dear ImGui's own behavior.

**Reduced motion:** `HelloImGui::PrefersReducedMotion()` tells whether the system asks for less motion: "Reduce motion" on macOS and iOS, "Show animations in Windows" turned off, a browser's `prefers-reduced-motion`. An application can then skip its animations. False where the system has no such setting (Linux, Android).

**Idling:** a mouse button (or a finger) held down keeps the app awake, even still: a button that repeats, a drag that pauses.

**Ini settings:** `LoadUserPref` gives back the value as saved by `SaveUserPref`. It came back with a trailing newline from the second run on: each read and write of the ini file added a `\n` to its last part.

**plutosvg / plutovg:**
* Now git submodules (`external/plutosvg`, with plutovg as its own submodule), updated to plutosvg v0.0.8 and plutovg v1.3.3. They are no longer downloaded at configure time, which also makes the release source archives complete. Both are compiled into a single static library `plutosvg`, part of the install.
* New option `HELLOIMGUI_USE_SYSTEM_PLUTOSVG`: link an installed plutosvg (found via `find_package(plutosvg CONFIG)` or pkg-config; it must be built with freetype support) instead of compiling the submodule. Defaults to ON when `IMGUI_BUNDLE_PYTHON_USE_SYSTEM_LIBS` is set. Packagers (conda, vcpkg, distributions) should use this.

**CMake package renamed to `hello_imgui`:**
* `find_package(hello_imgui)` and `hello_imgui::hello_imgui` replace `find_package(hello-imgui)` and `hello-imgui::hello_imgui`, so that the package, namespace and target share one name. The former name still works through a deprecated compatibility shim, to be removed in a future release. The config files are now installed in `lib/cmake/hello_imgui/`, where `find_package` can find them without vcpkg's config fixup. (The vcpkg port keeps its `hello-imgui` name, as vcpkg forbids underscores.)

# v1.92.901

Maintenance release (ImGui stays at v1.92.9b-docking). Fixes reported while packaging v1.92.900 for vcpkg ([vcpkg PR #54216](https://github.com/microsoft/vcpkg/pull/54216), [#169](https://github.com/pthom/hello_imgui/issues/169)).

**Fixes:**
* CMake: `project(VERSION)` was still 1.92.700 in the v1.92.900 tag, so the installed `hello-imguiConfigVersion.cmake` advertised the wrong version (#169)
* `ImGuiTheme::ApplyTheme` (and the theme list box) can again be used standalone, i.e. with an ImGui context but without `HelloImGui::Run()`. Since v1.92.6 they threw "HelloImGui::GetRunnerParams() would return null" (regression from the `ThemeChanged` callback)
* Ini settings: restore saved window positions with negative coordinates, i.e. a window on a monitor left of (or above) the primary one (#168)

**Licenses:**
* Add the license texts of the redistributed fonts next to them: `hello_imgui_assets/fonts/LICENSE-DroidSans.txt` (Apache-2.0) and `hello_imgui_assets/fonts/LICENSE-FontAwesome.txt` (OFL-1.1, Font Awesome 4 and 6)
* Add the MIT license text of the vendored inifile-cpp (`src/hello_imgui/internal/inicpp_LICENSE.txt`)

# v1.92.900

* Update ImGui to v1.92.9b-docking

**New Callbacks:**
* Add `BeforeSwap` callback: called after ImGui's draw data was rendered to the 3D backend, but before the frame is swapped to the screen. Enables a final full-screen post-process pass over the whole frame (e.g. a color-management pass), which is not possible with `BeforeImGuiRender` (too early: the draw data is not rendered yet) nor with `AfterSwap` (too late: the frame is already presented).
* Add `ConfirmExit` callback: called when the user requests to close the app window (window close button, Cmd-Q / Alt-F4, or the App/Quit menu). Return false to cancel the exit, e.g. to confirm quitting when there are unsaved changes.

**RendererBackendOptions:**
* `requestFloatBuffer` is now also supported by the OpenGL3 + Glfw backend (it used to be Metal-only), enabling HDR/EDR display output on Windows and Linux. This requires a GLFW that defines `GLFW_FLOATBUFFER` (no stock release does yet, but HDR-enabling forks such as https://github.com/Tom94/glfw do); it is silently ignored otherwise. HelloImGui now probes whether a floating point framebuffer can actually be created, and resets `requestFloatBuffer` to false if not, so applications can read the field back to know what they got. See the new `hello_edr_opengl` demo. Note: on macOS, EDR output still requires the Metal backend.

**HighDPI:**
* Fonts are now scaled via `style.FontScaleDpi` instead of at load time (finalizes the ImGui v1.92 transition). Breaking change: removes `FontLoadingParams::adjustSizeToDpi` and `DpiFontLoadingFactor()`

**Widgets:**
* `WidgetWithResizeHandle`: the handle color now uses `ImGuiCol_ResizeGrip` (with alphaMul=0.5)
* `InputTextResizable`: can now be resized even inside a node (node editor)

**Fixes:**
* `AssetFileFullPath` can find files in the current folder (was broken)
* Fix an MSVC internal compiler error on `plutovg-font.c` (compile it with /Od)

**Internals:**
* `ManualRender`: refactor behavior, simpler and closer to `ImmApp::ManualRender`


# v1.92.700

* Update ImGui to v1.92.7-docking

**RunnerParams:**
* Add `iniDisable` to `SimpleRunnerParams`: enables to never save/load prefs

**Textures & images:**
* Add public `TextureGpu` RAII handle (replaces internal `ImageAbstract`)
* Add `CreateTextureFromRgbaData` and `DeleteTexture`
* Add `ImageAndSizeFromEncodedData` and `LoadImageDataFromEncodedData` (decode images from in-memory buffers)
* Allow image / texture helpers to be used outside `HelloImGui::Run()` (adds `_EnsureGlLoaderForStandalone()` called by `_GetCachedImage`)

**Emscripten:**
* `IniFolderLocation` on emscripten now returns `""` for `CurrentFolder` (and `"/"` for other types)

**Fixes:**
* Skip `TearDown` if already ran at exit (e.g. if it threw an exception itself)
* `LoadDefaultFont_WithFontAwesomeIcons`: log if icon font not found

**Docs:**
* Document theming usage

**Internals**
* Add `emscriptenAllowBrowserZoomShortcuts` preference (true by default) to forward browser zoom shortcuts to the browser
* Add `_PrivIdleFrameWaitDurationForPythonAsyncIo` (internal, for Python async integration)


# v1.92.601

* Add `LoadImageDataFromAsset()` — decode an image from assets into CPU memory (C++ only), with configurable channel count

**Warnings hunt**
* Fix potential memory error in font handling (_LoadFontImpl: pass font buffer allocated with IM_ALLOC)
* Compile `imgui_impl_metal.mm` with `-fobjc-arc` (fixes ARC bridge cast warnings on macOS)
* Suppress Apple Clang `-Wdeprecated-declarations` warning in `imgui_freetype.cpp` (caused by `sprintf` in third-party `plutosvg-ft.h`)
* Update plutovg to v1.3.2 and plutosvg to v0.0.7
* Workaround plutovg `file(RELATIVE_PATH)` error when `CMAKE_INSTALL_PREFIX` is relative (scikit-build-core wheel builds)
* Normalize install path (`./lib/` → `lib`) to fix CMake CMP0177 warning
* cmake/assets: create asset destination dirs before `copy_if_different`

# v1.92.6

* Update ImGui to v1.92.6-docking

**New Callbacks:**

* Add callback PostNewFrame
* Add ThemeChanged callback (Fixes #156)

**New RunnerParams:**

* Added params iniDisable & iniClearPreviousSettings
* add topMost window attribute
* FpsIdling : Add configurable vsyncToMonitor + fpsMax. This introduces a new vsyncToMonitor field in FpsIdling, allowing users to
enable/disable synchronization with the monitor refresh rate.
* Added params to control ini file settings
```cpp
// `iniDisable`: _bool, default = false_. If true, do not save or load any settings to or from an ini file.
bool iniDisable = false;
// `iniClearPreviousSettings`: _bool, default = false_. If true, delete any previous settings ini file at application startup.
bool iniClearPreviousSettings = false;
```

**Emscripten** 
  * add loading spinner overlay to default shell template
  * use Glfw3 by default instead of Sdl2. We use pongasoft/emscripten-glfw under emscripten, which has good support for the clipboard

**Fixes:**
* Fix: ManualRender RunnerParams lifetime management 
    Issue: https://github.com/pthom/imgui_bundle/issues/417
* RaiseWindow at startup (improve SDL initial show on macOS)

**Assets Handling:**
* Add AddAssetsSearchPath to allow adding additional search paths for assets (e.g. to load assets from a different location, or to load additional assets without modifying the original assets folder)



# v1.92.5
* Update ImGui to v1.92.5-docking
* InputTextResizable: fix for node editor (can't resize multiline edit when inside a node)
* AbstractRunner: improve non idling detection
* Emscripten: can also run using glfw3 (via pongasoft/emscripten-glfw)
* windows: Support icon.ico in _hello_imgui_add_windows_icon

# v1.92.3
* toolbars: fix issue in toolbar min size (thanks @wkjarosz)
### Assets
* Separate ImageFromAsset and ImageFromAssetWithBg: use ImageFromAssetWithBg to display images with a background or border.
* Add `SetLoadAssetFileDataFunction(LoadAssetFileDataFunc func)` (Redirect asset loads to user-defined function). Thanks to Jorg Neves Bliesener
* LoadFontTTF_WithFontAwesomeIcons: can load FontAwesome 6
### cmake
* Add cmake option IMGUI_DISABLE_OBSOLETE_FUNCTIONS
* cmake: can use plutosvg without downloading (using HELLOIMGUI_DOWNLOAD_FREETYPE_IF_NEEDED=OFF)

# v1.92.0
Version numbers are now synced between "Dear ImGui" "Hello ImGui" and "Dear ImGui Bundle".

* ImGui: Many Font related changes: this release brings many changes on the ImGui side (v1.92.0): do read the [release notes for ImGui v1.92.0](https://github.com/ocornut/imgui/releases/tag/v1.92.0)
TLDR: Fonts may be rendered at any size. Glyphs are loaded and rasterized dynamically. No need to specify ranges, prebake etc. 

* Removed FontDpiResponsive: this is now handled by ImGui itself
* Removed FontLoadingParams.glyphRanges, since ranges are not needed anymore by ImGui

# v1.6.3
- Assets: can search with absolute path or from current working directory
- Add utility function void UseWindowFullMonitorWorkArea()
- AppWindowParams: add EmscriptenKeyboardElement
- Runner: call TearDown on Setup for Python (to make it possible to recover from exceptions in notebook)
- MakeWindowSizeRelativeTo96Ppi_IfRequired: call EnsureWindowFitsThisMonitor
- update example_integration (Add CMake example / GNU Install)
- Add cmake option HELLOIMGUI_USE_EXTERNAL_JSON (to provide nlohmann json yourself)
- compatibility with CMake 4

# v1.6.0
* SVG Font rendering: plutosvg replaces lunasvg (option HELLOIMGUI_USE_FREETYPE_PLUTOSVG on by default)
* Added AddDockableWindow / RemoveDockableWindow
* demo_docking: better demonstration / theme customization
* Add `HelloImGui::ManualRender: a namespace that groups functions, allowing fine-grained control over the rendering process
* Work on pyodide integration (for ImGui Bundle)
* Improve font rendering on iOS


# v1.5.0
* Improved rendering on Windows (via antialiasing)
* Add IniFolderType.AbsolutePath
* Polish themes
* Add callback PostRenderDockableWindows
* Added optional [hello_imgui.ini](https://github.com/pthom/hello_imgui/blob/master/hello_imgui_example.ini) file, 
  as a way to do advanced configuration for Dpi and OpenGL rendering options
* Add PushTweakedTheme / PopTweakedTheme (different ImGui windows can use a different theme. See demo docking)
* Add DpiAwareParams: see [doc](https://pthom.github.io/hello_imgui/book/doc_params.html#dpi-aware-params)
* Add HelloImGui::ImageAndSizeFromAsset see [doc](https://pthom.github.io/hello_imgui/book/doc_api.html#display-images-from-assets)
* callbacks.LoadAdditionalFonts can be modified and reused during execution
* add null backend (see https://pthom.github.io/hello_imgui/book/doc_params.html#backend-selection)
* Add [optional OpenGlOptions](https://github.com/pthom/hello_imgui/blob/98f241df2b43bb2b3191ab150c2b1a21b30c7031/src/hello_imgui/renderer_backend_options.h#L9-L56) 
* Add widgets InputTextResizable + WidgetWithResizeHandle: see [doc](https://pthom.github.io/hello_imgui/book/doc_api.html#additional-widgets)
* Add HelloImGui::LoadDpiResponsiveFont: see [doc](https://pthom.github.io/hello_imgui/book/doc_api.html#load-fonts)

#### Add FontAwesome options with support for FontAwesome 4 and FontAwesome 6:

Breaking change: you need to include manually the icons: `#include "hello_imgui/icons_font_awesome_4.h"`
The default icon font is FontAwesome 4 (for backward compatibility), but v6 includes many more icons

In order to select Font Awesome 6, you need to set the following in your runnerParams:
```cpp
 runnerParams.runnerCallbacks.defaultIconFont = hello_imgui::IconFont::FontAwesome6;
```
and then include:
```cpp
#include "hello_imgui/icons_font_awesome_6.h"
 ```

# v1.4.2

* Integration with vcpkg ongoing (see [PR](https://github.com/microsoft/vcpkg/pull/36501))
* Reviewed CMake backend selection process
You can now build with several rendering backends and several platform backends at the same time.
* Work on vcpk packaging

# v1.4.0

## Vcpkg support for dependencies
You can install almost all required dependencies with [vcpkg](https://github.com/microsoft/vcpkg). 
```bash
# Clone hello_imgui 
git clone https://github.com/pthom/hello_imgui.git
cd hello_imgui
# Clone vcpkg -& bootstrap
git clone https://github.com/microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh
# Install dependencies required by hello_imgui
./vcpkg/vcpkg install "glad[gl-api-43] stb freetype lunasvg glfw3 sdl2 imgui[opengl3-binding, docking-experimental, glfw-binding, sdl2-binding,freetype, freetype-lunasvg]"
# Build hello_imgui
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . -j 4 
```

Notes: 
- this will not support ImGui Test Engine, as it is not available in vcpkg yet.
- See CI Tests: [![VcpkgDeps](https://github.com/pthom/hello_imgui/workflows/VcpkgDeps/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/VcpkgDeps.yml)


## Vcpkg packaging
hello_imgui is now ready to be integrated to [vcpkg](https://github.com/microsoft/vcpkg).

You can test this with the following commands:
```bash
# Clone hello_imgui  (just to get the overlay in hello_imgui_cmake/overlay_vcpkg/hello-imgui)
git clone https://github.com/pthom/hello_imgui.git
cd hello_imgui
# Clone vcpkg -& bootstrap
git clone https://github.com/microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh
# Install hello_imgui via vcpkg (using the overlay)
./vcpkg/vcpkg install "hello-imgui[opengl3-binding, glfw-binding, sdl2-binding]" --overlay-ports=hello_imgui_cmake/overlay_vcpkg/hello-imgui
```

Notes:
- See CI Tests: [![VcpkgPackage](https://github.com/pthom/hello_imgui/workflows/VcpkgPackage/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/VcpkgPackage.yml)

## Other
* Update update imgui to  v1.90.1-docking
* Add demo + doc / FontAwesome 6


# v1.3.0

## New Features

* Added EdgeToolbars: see [definition](https://github.com/pthom/hello_imgui/blob/3a279ce7459b04a4c2e7460b844cbf354833964e/src/hello_imgui/runner_callbacks.h#L72-L102), [callbacks](https://github.com/pthom/hello_imgui/blob/3a279ce7459b04a4c2e7460b844cbf354833964e/src/hello_imgui/runner_callbacks.h#L140-L147), [example usage](https://github.com/pthom/hello_imgui/blob/3a279ce7459b04a4c2e7460b844cbf354833964e/src/hello_imgui_demos/hello_imgui_demodocking/hello_imgui_demodocking.main.cpp#L694-L714), and [demo](https://imgui-bundle.pages.dev/explorer/demo_docking.html)
* Callbacks: add [EnqueuePostInit, EnqueueBeforeExit, PostInit_AddPlatformBackendCallbacks](https://pthom.github.io/hello_imgui/book/doc_params.html#runnercallbacks)
* Add [renderer_backend_options](https://pthom.github.io/hello_imgui/book/doc_params.html#renderer-backend-options)
* Add support for Extended Dynamic Range (EDR) on macOS : see [PR](https://github.com/pthom/hello_imgui/pull/89). Added [demo / EDR](https://github.com/pthom/hello_imgui/tree/master/src/hello_imgui_demos/hello_edr) - Only works with Metal
* Test Engine: can re-call params.callbacks.RegisterTests
* rememberEnableIdling default=false (true is too surprising)
* emscripten: Use webgl2 / GLES3

# Other, between v1.0.0 and v1.3.0

* Added nice [documentation pages](https://pthom.github.io/hello_imgui)
* Uses [Freetype for font rendering](https://github.com/pthom/hello_imgui/blob/549c205dd3ca98f18fcf541a2ebbfc5abdd10410/CMakeLists.txt#L96-L106)
* Improved [Font Loading utility](https://github.com/pthom/hello_imgui/blob/549c205dd3ca98f18fcf541a2ebbfc5abdd10410/src/hello_imgui/hello_imgui_font.h#L13-L62)
* Added support for Colored font and Emoji fonts ([Demo](https://imgui-bundle.pages.dev/explorer/demo_docking.html))
* Can [fully customize the menu bar](https://pthom.github.io/hello_imgui/book/doc_api.html#customize-hello-imgui-menus)
* Added support for macOS application bundles
* Added option to specify where settings are saved: `RunnerParams.iniFolderType` can be set to: `CurrentFolder`, `AppUserConfigFolder`, `DocumentsFolder`, `HomeFolder`, `TempFolder`, `AppExecutableFolder`.
* Support for Application Icon: the file `assets/app_settings/icon.png` will be used to generate the window icon (C++, Python), and app icon (C++ only) for any platform. See assets structure below:
```
assets/
├── world.png                         # A custom asset
├── app_settings/                     # Application settings
│         ├── icon.png                # This will be the app icon, it should be square
│         │                           # and at least 512x512. It will  be converted
│         │                           # to the right format, for each platform.
│         ├── apple/
│         │         └── Info.plist    # macOS and iOS app settings
│         │                          # (or Info.ios.plist + Info.macos.plist)
├── fonts/
│         ├── DroidSans.ttf               # Default fonts
│         └── fontawesome-webfont.ttf     #     used by HelloImGui
│         ├── Roboto
│         │    ├── Roboto-Bold.ttf        # Font used by Markdown
│         │    ├── Roboto-BoldItalic.ttf
│         │    ├── Roboto-Regular.ttf
│         │    └── Roboto-RegularItalic.ttf
│         ├── SourceCodePro-Regular.ttf
├── images
│         └── markdown_broken_image.png
```
* hello_imgui_add_app and imgui_bundle.add_app can now accept ASSETS_LOCATION as a parameter e.g. `hello_imgui_add_app(my_app file1.cpp file2.cpp ASSETS_LOCATION my_assets)`


# v1.0.0

* Integrated ImGui Test Engine
* Layout & docking: can switch between several layout and save their settings separately
* Reduce FPS when not in use to save CPU usage: added FpsIdling options
* Can store custom user preferences
* Greatly improved HighDpi support
* Improved Theming and Themes
* Added callbacks: PostInit / BeforeExit_PostCleanup / PreNewFrame
* MingW compatibility (and CI)
* Make it possible to run without assets/fonts
* Improved emscripten multithread support
