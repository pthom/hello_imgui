# Assets & App customization

Anything in the assets/ folder located beside the app's CMakeLists will be embedded
on mobile devices and emscripten, i.e. they will be bundled together with the app;
and you can access them via `assetFileFullPath(const std::string& assetRelativeFilename)`.

Typical content of the assets/ folder:

The default fonts are available inside [hello_imgui_assets](https://github.com/pthom/hello_imgui/tree/master/hello_imgui_assets).


# App customization

The subfolder `assets/app_settings` located beside your app's CMakeLists can store some settings: app icon, app name, etc.

Example:
```
assets/
├── app_settings/
│    ├── icon.png          # If present, this will be used as the app icon.
│    │                     # Tt should be a square image, of large size, at least 256x256.
│    │                     # This works on macOS, iOS, windows, linux, and emscripten 
│    │                     # and it will automatically 
│    │                     # be converted to the correct format.
│    │
│    ├── apple/
│    │    ├── Info.plist    # Will be used as the app Info.plist for iOS and macOS
│    │    │                      # (if absent, HelloImGui will provide a default one)
│    │    │
│    │    ├── Info.ios.plist     # (optional, used on iOS if present)
│    │    ├── Info.macos.plist   # (optional, used on macOS if present) 
│    │ 
│    └── emscripten/
│        ├── shell.emscripten.html # Emscripten shell file
│        │                         #   (this file will be cmake "configured"
│        │                         #    to add the name and favicon) 
│        └── custom.js             # Any custom file here will be deployed
│                                  #   in the emscripten build folder
│
├── fonts/
│    ├── DroidSans.ttf                 # default fonts 
│    └── fontawesome-webfont.ttf  # used by HelloImGui
├── images/
│      ├── whatever.jpg             # add any file you like in whatever folder you like

```
