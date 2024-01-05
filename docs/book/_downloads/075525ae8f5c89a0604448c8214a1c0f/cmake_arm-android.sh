#!/usr/bin/env bash

# Assert we have one parameter and set source_dir to it
if [ $# -ne 1 ]; then
  echo "Usage: $0 <source_dir>"
  exit 1
fi
export source_dir=$1

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

export android_abi=armeabi-v7a
$THIS_DIR/_impl_cmake_android.sh
