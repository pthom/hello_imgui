#!/usr/bin/env bash

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
$THIS_DIR/../sdl_download.sh

cd $THIS_DIR/../../external
cd SDL2-2.0.12/Xcode-iOS/SDL

# Build for iphone
xcodebuild -configuration Release -project SDL.xcodeproj -target libSDL-iOS

# Build for simulator
xcodebuild -configuration Release -project SDL.xcodeproj -target libSDL-iOS -arch x86_64 -sdk iphonesimulator
