#!/usr/bin/env bash

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $THIS_DIR/../.. || exit 1

# See https://github.com/microsoft/vcpkg/blob/master/docs/users/android.md
export android_abi=arm64-v8a
export vcpkg_target_triplet=arm64-android

build_dir="build_arm64-android"
if [ ! -d $build_dir ]; then
  mkdir $build_dir
fi

export source_dir=$THIS_DIR/../..

cd $build_dir
$THIS_DIR/_impl_cmake_android.sh
