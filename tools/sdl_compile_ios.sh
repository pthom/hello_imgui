#!/usr/bin/env bash

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $THIS_DIR/../external/SDL/Xcode-iOS/SDL
xcodebuild -target libSDL-iOS  -project SDL.xcodeproj
