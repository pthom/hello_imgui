#!/usr/bin/env bash

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
export source_dir=$THIS_DIR/../..

# See https://github.com/microsoft/vcpkg/blob/master/docs/users/android.md
export android_abi=armeabi-v7a
export vcpkg_target_triplet=arm-android

$THIS_DIR/_impl_cmake_android.sh
