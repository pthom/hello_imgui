#!/usr/bin/env bash

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $THIS_DIR/external

wget https://www.libsdl.org/release/SDL2-2.0.12.tar.gz
tar xvfz SDL2-2.0.12.tar.gz
rm SDL2-2.0.12.tar.gz
ln -s SDL2-2.0.12 SDL
cd SDL2-2.0.12/Xcode-iOS/SDL
xcodebuild -target libSDL-iOS  -project SDL.xcodeproj

