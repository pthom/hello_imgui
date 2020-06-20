#!/usr/bin/env bash

# See https://github.com/microsoft/vcpkg/blob/master/docs/users/android.md

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

if [ -z "$ANDROID_NDK_HOME" ]; then
  echo "Please set env variable ANDROID_NDK_HOME"
  exit 1
fi

if [ -z "$JAVA_HOME" ]; then
  echo "Please set env variable JAVA_HOME (jdk version 8 required)"
  exit 1
fi

if [ -z "$vcpkg_target_triplet" ]; then
  echo "Please set env variable $vcpkg_target_triplet"
  exit 1
fi

if [ -z "$android_abi" ]; then
  echo "Please set env variable android_abi"
  exit 1
fi

if [ -z "$source_dir" ]; then
  echo "Please set env variable source_dir"
  exit 1
fi

export VCPKG_ROOT=$THIS_DIR/../../vcpkg
export vcpkg_toolchain_file=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
export android_toolchain_file=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake
export PATH=$JAVA_HOME/bin:$PATH

cmd="cmake \
    -DCMAKE_TOOLCHAIN_FILE=$vcpkg_toolchain_file \
    -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=$android_toolchain_file \
    -DVCPKG_TARGET_TRIPLET=$vcpkg_target_triplet \
    -DANDROID_ABI=$android_abi \
    -DHELLOIMGUI_USE_SDL_OPENGL3=ON \
    $source_dir
    "

echo $cmd
$cmd
