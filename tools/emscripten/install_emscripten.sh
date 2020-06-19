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
