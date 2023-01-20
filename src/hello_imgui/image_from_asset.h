#pragma once
#include "imgui.h"

/**
@@md#HelloImGui::ImageFromAsset

* `HelloImGui::ImageFromAsset(const char *assetPath, size, ...)`: will display a static image from the assets.
* `bool HelloImGui::ImageButtonFromAsset(const char *assetPath, size, ...)`: will display a button using an image from the assets.
* `ImTextureID HelloImGui::ImTextureIdFromAsset(const char *assetPath)`: will return a texture ID for an image loaded from the assets.

Images are loaded when first displayed, and then cached (they will be freed just before the application exits).

For example, given this files structure:
```
├── CMakeLists.txt
├── assets/
│   └── my_image.jpg
└── my_app.main.cpp
```

then, you can display "my_image.jpg", using:

```cpp
HelloImGui::ImageFromAsset("my_image.jpg");
```

*Note: HelloImGui::ImageFromAsset only works with OpenGL backends. It will throw an exception on other backends*

@@md
*/

namespace HelloImGui
{
    void ImageFromAsset(const char *assetPath, const ImVec2& size = ImVec2(0, 0), const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1,1), const ImVec4& tint_col = ImVec4(1,1,1,1), const ImVec4& border_col = ImVec4(0,0,0,0));
    bool ImageButtonFromAsset(const char *assetPath, const ImVec2& size = ImVec2(0, 0), const ImVec2& uv0 = ImVec2(0, 0),  const ImVec2& uv1 = ImVec2(1,1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0,0,0,0), const ImVec4& tint_col = ImVec4(1,1,1,1));
    ImTextureID ImTextureIdFromAsset(const char *assetPath);

    namespace internal
    {
        void Free_ImageFromAssetMap();
    }
}
