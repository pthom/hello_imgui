#!/usr/bin/env bash

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

$THIS_DIR/_impl_vcpkg_create_android_triplets.sh
cd $THIS_DIR/../../vcpkg
./vcpkg install sdl2:arm-android sdl2:arm64-android
