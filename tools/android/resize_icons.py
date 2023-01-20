#!/usr/bin/env python3

# python3 -m pip install --upgrade Pillow
import os
import sys
from PIL import Image

doc_String = """
@@md#resize_icons

This script will create several android icons with correct size.

Your app folder should look like this:

```
your_app/
├── CMakeLists.txt
├── android/                  # Run this script from this folder
│   └── mipmap-source/
│       └── ic_launcher.png   # Put here a big version of your icon
├── assets/
├── hello_imgui_demodocking.main.cpp
└── ios/
```

Run this script from the subfolder android/ of your app folder.
A folder named mipmap-source should be present in it, with an icon ic_launcher.png inside it


When running this script, several variations of the icons will be created:
```
your_app/
├── CMakeLists.txt
├── android/
│   ├── mipmap-source/
│   │   └── ic_launcher.png
│   └── res/
│       ├── mipmap-hdpi/
│       │   └── ic_launcher.png
│       ├── mipmap-mdpi/
│       │   └── ic_launcher.png
│       ├── mipmap-xhdpi/
│       │   └── ic_launcher.png
│       ├── mipmap-xxhdpi/
│       │   └── ic_launcher.png
│       └── mipmap-xxxhdpi/
│           └── ic_launcher.png
├── assets/
│   └── fonts/
│       └── Akronim-Regular.ttf
├── hello_imgui_demodocking.main.cpp
└── ios/
```

@@md
"""

SOURCE_ICON = "mipmap-source/ic_launcher.png"

if not os.path.isfile(SOURCE_ICON):
    print(
        """
    This script will create several android icons with correct size.
    Please run it from the subfolder android/res of your app folder.
    A folder named mipmap-source should be present in it, with an 
    icon ic_launcher.png inside it
    """
    )
    sys.exit(1)


icons_folders_and_size = {
    "mipmap-hdpi": 72,
    "mipmap-mdpi": 48,
    "mipmap-xhdpi": 96,
    "mipmap-xxhdpi": 144,
    "mipmap-xxxhdpi": 192,
}


for folder, size in icons_folders_and_size.items():
    print(f"{folder}:{size}")
    if not os.path.isdir("res"):
        os.mkdir("res")
    dst_folder = "res/" + folder
    if not os.path.isdir(dst_folder):
        os.mkdir(dst_folder)
    source_image = Image.open(SOURCE_ICON)
    source_image.thumbnail((size, size))
    source_image.save(dst_folder + "/ic_launcher.png", "PNG")
