#!/usr/bin/env bash

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $THIS_DIR/../.. || exit 1


build_dir="build_emscripten"
if [ ! -d $build_dir ]; then
  mkdir $build_dir
fi


cd $build_dir
emcmake cmake .. -DHELLOIMGUI_USE_SDL_OPENGL3=ON ..
