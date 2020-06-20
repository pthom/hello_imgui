#!/usr/bin/env bash

import os
import subprocess

THIS_DIR=os.path.dirname(os.path.realpath(__file__))
os.chdir(THIS_DIR + "/..")

if not os.path.isdir("vcpkg"):
    subprocess.check_call("git clone https://github.com/Microsoft/vcpkg.git")
    os.chdir("vcpkg")

    if os.name == 'nt':
        subprocess.check_call("bootstrap-vcpkg.bat")
    else:
        subprocess.check_call("bash ./bootstrap-vcpkg.sh")

    if os.name == 'nt':
        subprocess.check_call("vcpkg.exe install glfw3 sdl2")
    else:
        subprocess.check_call("./vcpkg install glfw3 sdl2")
