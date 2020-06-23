#!/usr/bin/env bash

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
REPO_DIR=$THIS_DIR/../..

cd $build_dir
emcmake cmake -DHELLOIMGUI_USE_SDL_OPENGL3=ON $REPO_DIR
