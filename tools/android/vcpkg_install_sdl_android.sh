#!/usr/bin/env bash

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $THIS_DIR/../..

git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
$THIS_DIR/_impl_vcpkg_create_android_triplets.sh
./vcpkg install sdl2:arm-android sdl2:arm64-android || true

echo ""
echo ""
echo ""
echo "/home/runner/work/hello_imgui/hello_imgui/vcpkg/buildtrees/sdl2/config-arm-android-dbg-out.log"
cat  /home/runner/work/hello_imgui/hello_imgui/vcpkg/buildtrees/sdl2/config-arm-android-dbg-out.log

echo ""
echo ""
echo ""
echo "/home/runner/work/hello_imgui/hello_imgui/vcpkg/buildtrees/sdl2/config-arm-android-dbg-err.log"
cat /home/runner/work/hello_imgui/hello_imgui/vcpkg/buildtrees/sdl2/config-arm-android-dbg-err.log
