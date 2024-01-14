#!/bin/bash

SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]:-$0}"; )" &> /dev/null && pwd 2> /dev/null; )";

REPO_DIR=$SCRIPT_DIR/..
echo REPO_DIR=$REPO_DIR
PYTHON_BIN_DIR=$REPO_DIR/venv/bin/
if [ ! -d $PYTHON_BIN_DIR ]; then
    echo "ERROR: Python virtual environment not found. Please create a virtual environment in REPO_DIR/venv (with jupyter-book installed)"
    exit 1
fi

cd $SCRIPT_DIR
rm -rf _build
$PYTHON_BIN_DIR/jupyter-book build .
