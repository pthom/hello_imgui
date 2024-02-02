#!/usr/bin/env bash

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

if [ -z "$ANDROID_NDK_HOME" ]; then
  if [ ! -z "$ANDROID_HOME" ]; then
    echo "trying android home"
    if [ -d $ANDROID_HOME/ndk-bundle ]; then
      export ANDROID_NDK_HOME=$ANDROID_HOME/ndk-bundle
    fi
  fi
fi

if [ -z "$ANDROID_NDK_HOME" ]; then
  echo "Please set env variable ANDROID_NDK_HOME"
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

export android_toolchain_file=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake
export PATH=$JAVA_HOME/bin:$PATH

cmd="cmake \
    -DCMAKE_TOOLCHAIN_FILE=$android_toolchain_file \
    -DANDROID_ABI=$android_abi \
    -DHELLOIMGUI_USE_SDL2=ON \
    -DHELLOIMGUI_HAS_OPENGL3=ON \
    -DHELLOIMGUI_CREATE_ANDROID_STUDIO_PROJECT=ON \
    $source_dir
    "

echo $cmd
$cmd
