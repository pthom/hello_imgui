#!/usr/bin/env bash

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
REPO_DIR=$THIS_DIR/../../..
cd $REPO_DIR

if [ ! -d build_emscripten ]; then
  mkdir build_emscripten
fi

cd build_emscripten
source ~/emsdk/emsdk_env.sh
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
make -j 4
