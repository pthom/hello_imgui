#!/usr/bin/env bash

# export VCPKG_ROOT=$(pwd)/../../../vcpkg

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
echo "SCRIPT_DIR: $SCRIPT_DIR"

# trap errors
trap 'echo "Error occurred on line $LINENO"; exit 1' ERR


overlay_dir=$SCRIPT_DIR/../../overlay_vcpkg/hello-imgui

function vcpkg_reinstall() {
    $VCPKG_ROOT/vcpkg remove hello-imgui
    $VCPKG_ROOT/vcpkg install "hello-imgui[glfw-binding, sdl2-binding, $current_backend_binding]" --overlay-ports=$overlay_dir --debug --no-binarycaching --recurse
}

function do_build() {
    cd $SCRIPT_DIR
    rm -rf build
    mkdir -p build
    cd build
    cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
    cmake --build .
    cd $SCRIPT_DIR
}

function do_run_gui_test() {
  cd $SCRIPT_DIR/build
  ./test
  cd $SCRIPT_DIR
}

function do_all() {
  vcpkg_reinstall
  do_build
  do_run_gui_test
}

function choose_rendering_backend() {
  # current_backend_binding can take the following values:
  #opengl3-binding
  #metal-binding
  #vulkan-binding
  #dx11-binding
  #dx12-binding

  # ask the user to choose a rendering backend
  echo "Choose a rendering backend:"
  echo "1) opengl3-binding"
  echo "2) metal-binding"
  echo "3) vulkan-binding"
  echo "4) dx11-binding"
  echo "5) dx12-binding"
  read -p "Enter your choice: " choice
  case $choice in
    1)
      current_backend_binding="opengl3-binding"
      ;;
    2)
      current_backend_binding="metal-binding"
      ;;
    3)
      current_backend_binding="vulkan-binding"
      ;;
    4)
      current_backend_binding="dx11-binding"
      ;;
    5)
      current_backend_binding="dx12-binding"
      ;;
    *)
      echo "Invalid choice"
      exit 1
      ;;
  esac
}


choose_rendering_backend
do_all

