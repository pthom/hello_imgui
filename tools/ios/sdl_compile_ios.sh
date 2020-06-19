#!/usr/bin/env bash

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $THIS_DIR/../../external || exit 1

wget https://www.libsdl.org/release/SDL2-2.0.12.tar.gz
tar xvfz SDL2-2.0.12.tar.gz
rm SDL2-2.0.12.tar.gz
ln -s SDL2-2.0.12 SDL
cd SDL2-2.0.12/Xcode-iOS/SDL

# Build for iphone
xcodebuild -configuration Release -project SDL.xcodeproj -target libSDL-iOS

# Build for simulator
xcodebuild -configuration Release -project SDL.xcodeproj -target libSDL-iOS -arch x86_64 -sdk iphonesimulator
