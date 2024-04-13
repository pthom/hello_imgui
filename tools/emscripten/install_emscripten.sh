#!/usr/bin/env bash

cd $HOME

if [ ! -d emsdk ]; then
  git clone https://github.com/emscripten-core/emsdk.git
fi

cd emsdk

# Fetch the latest version of the emsdk (not needed the first time you clone)
git pull

# Download and install the latest SDK tools.
./emsdk install latest

# Make the "latest" SDK "active" for the current user. (writes .emscripten file)
./emsdk activate latest

# Activate PATH and other environment variables in the current terminal
source ~/emsdk/emsdk_env.sh

# Exclude version 3.1.57 (see https://github.com/emscripten-core/emscripten/issues/21756)
output=$(emcc -v 2>&1 | grep -c 3.1.57)
if [ $output -eq 1 ]
then
  ./emsdk install 3.1.56
  ./emsdk activate 3.1.56
  source ~/emsdk/emsdk_env.sh
fi
