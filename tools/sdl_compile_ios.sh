#!/usr/bin/env bash

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $THIS_DIR/external

wget https://www.libsdl.org/release/SDL2-2.0.12.tar.gz
tar xvfz SDL2-2.0.12.tar.gz
rm SDL2-2.0.12.tar.gz
ln -s SDL2-2.0.12 SDL
cd SDL2-2.0.12/Xcode-iOS/SDL
#xcodebuild  -project SDL.xcodeproj
echo "For some reason, SDL build by command line (xcodebuild) fails."
echo "Please build it SDL via XCode"
echo "I will now open XCode : please click the menu Product, then Build."
echo "You can quit XCode after this."
read -p "Press enter to continue"
open SDL.xcodeproj
