#!/usr/bin/env bash

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $THIS_DIR/../external || exit 1

if [ ! -d SDL2-2.28.5 ]; then
  wget https://www.libsdl.org/release/SDL2-2.28.5.tar.gz
  tar xvfz SDL2-2.28.5.tar.gz
  rm SDL2-2.28.5.tar.gz
  ln -s SDL2-2.28.5 SDL
fi
