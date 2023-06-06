#!/usr/bin/env bash

# This script is for authors only (it deploys the emscripten demos onto the server)
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
REPO_DIR=$THIS_DIR/../../..
cd $REPO_DIR

./tools/emscripten/deploy/emscripten_build.sh
cd build_emscripten
rsync -vaz bin pascal@traineq.org:HTML/HelloImGui/
