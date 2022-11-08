#!/usr/bin/env python

import os
import subprocess

THIS_DIR = os.path.dirname(os.path.realpath(__file__))
os.chdir(THIS_DIR + "/..")


def run(cmd):
    subprocess.check_call(cmd, shell=True)


if not os.path.isdir("vcpkg"):
    run("git clone https://github.com/Microsoft/vcpkg.git")
    os.chdir("vcpkg")

    if os.name == "nt":
        run("bootstrap-vcpkg.bat")
    else:
        run("./bootstrap-vcpkg.sh")

    if os.name == "nt":
        run("vcpkg.exe install glfw3:x86-windows sdl2:x86-windows")
        run("vcpkg.exe install glfw3:x64-windows sdl2:x64-windows")
        # run("vcpkg.exe install glfw3:arm64-windows sdl2:arm64-windows")
    else:
        run("./vcpkg install glfw3 sdl2")
