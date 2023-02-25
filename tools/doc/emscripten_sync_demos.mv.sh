#!/usr/bin/env bash

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $THIS_DIR/../.. || exit 1

source ~/emsdk/emsdk_env.sh
mkdir -p build_emscripten
cd build_emscripten/
emcmake cmake ..
make -j 4

cd src
rsync -vaz hello_imgui_demos pascal@traineq.org:HTML/HelloImGui
