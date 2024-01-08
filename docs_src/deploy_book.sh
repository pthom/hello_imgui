#!/bin/bash

SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]:-$0}"; )" &> /dev/null && pwd 2> /dev/null; )";

REPO_DIR=$SCRIPT_DIR/..

# Exit on error
set -e
# Echo each command
#set -x

cd "$REPO_DIR" || exit

echo "Checking status of the repository"
echo "=================================================="

# Check that everything is committed
if [[ -n $(git status -s --untracked-files=no) ]]; then
    echo "Please commit all changes before deploying"
    git status -s --untracked-files=no
    exit
fi
echo "OK, all changes are committed; master in sync with origin"

# Remember current branch and check that we are on master
CURRENT_BRANCH=$(git branch --show-current)
if [[ "$CURRENT_BRANCH" != "master" ]]; then
    echo "Please deploy from master"
    exit
fi
echo "OK, we are on the master branch"

# Check that master is up to date with origin
git fetch
LOCAL=$(git rev-parse @)
REMOTE=$(git rev-parse @{u})
BASE=$(git merge-base @ @{u})
if [[ $LOCAL != $REMOTE ]]; then
    echo "Please update your master branch"
    exit
fi
echo "OK, master is up to date with origin"


echo "Processing docs"
echo "=================================================="

# Checkout docs branch and merge master into it
git checkout docs
git merge master
echo "- merged master branch into docs branch"


# Build docs
# Run REPO_DIR/tools/doc/process_md_docs.py
# using python from the virtual environment venv
# Check python presence in the venv/ directory
if [[ ! -f "$REPO_DIR/venv/bin/python" ]]; then
    echo "Please create the virtual environment first"
    exit
fi
# Activate the virtual environment
source "$REPO_DIR/venv/bin/activate"
# Run the script
python "$REPO_DIR/tools/doc/process_md_docs.py"
echo "- built docs ($REPO_DIR/tools/doc/process_md_docs.py)"

# commit with message "Update docs / date / time"
git add docs/book/
git commit -m "Update docs / $(date '+%Y-%m-%d') / $(date '+%H:%M:%S')"
git push
echo "- committed and pushed docs"

# Checkout master branch
git checkout master
echo "- checked out master branch"
