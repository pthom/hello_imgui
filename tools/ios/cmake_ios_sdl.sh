#!/usr/bin/env bash

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $THIS_DIR/../.. || exit 1

if [ -z "$CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM" ]; then
  echo "Please set env variable CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM"
  exit 1
fi

build_dir="build_ios_sdl"
if [ ! -d $build_dir ]; then
  mkdir $build_dir
fi

cd $build_dir
cmake .. \
  -GXcode \
  -DCMAKE_TOOLCHAIN_FILE=../hello_imgui_cmake/ios-cmake/ios.toolchain.cmake \
  -DHELLOIMGUI_USE_SDL_OPENGL3=ON \
  -DPLATFORM=OS64 \
  -DENABLE_BITCODE=OFF \
  .. \
  || exit 1

open HelloImGui.xcodeproj
