#!/usr/bin/env bash

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $THIS_DIR/..

if [ ! -d vcpkg ]; then
  git clone https://github.com/Microsoft/vcpkg.git
  cd vcpkg
  ./bootstrap-vcpkg.sh
  ./vcpkg install glfw3
fi
