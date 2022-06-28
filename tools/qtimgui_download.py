#!/usr/bin/env python
import os
import subprocess


def run(cmd):
    subprocess.check_call(cmd, shell=True)


this_dir = os.path.dirname(os.path.realpath(__file__))
os.chdir(this_dir + "/../external")
if not os.path.isdir("qtimgui"):
    run("git clone https://github.com/seanchas116/qtimgui.git")
    os.chdir("qtimgui")
    run("git checkout 8666384bc5c6cb0b6b415b27c648375f734c2366")
