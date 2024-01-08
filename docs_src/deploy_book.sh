#!/bin/bash

# This script will update the docs inside docs/book/ in the docs branch
# and push the changes to the remote repository

SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]:-$0}"; )" &> /dev/null && pwd 2> /dev/null; )";
REPO_DIR=$SCRIPT_DIR/..

# Exit on error
#set -e
# Echo each command
#set -x

cd "$REPO_DIR" || exit


echo "=================================================="
echo "Checking status of the repository"
echo "=================================================="

# Check that everything is committed
if [[ -n $(git status -s --untracked-files=no) ]]; then
    echo "Please commit all changes before deploying"
    git status -s --untracked-files=no
    exit 1
fi
echo "OK, all changes are committed; master in sync with origin"

# Remember current branch and check that we are on master
CURRENT_BRANCH=$(git branch --show-current)
if [[ "$CURRENT_BRANCH" != "master" ]]; then
    echo "Please deploy from master"
    exit 1
fi
echo "OK, we are on the master branch"

# Check that master is up to date with origin
#git fetch
#LOCAL=$(git rev-parse @)
#REMOTE=$(git rev-parse @{u})
#BASE=$(git merge-base @ @{u})
#if [[ $LOCAL != $REMOTE ]]; then
#    echo "Please update your master branch"
#    exit 1
#fi
#echo "OK, master is up to date with origin"

# Check that master is up to date with origin or ahead of it
git fetch
LOCAL=$(git rev-parse @)
REMOTE=$(git rev-parse @{u})
BASE=$(git merge-base @ @{u})

if [[ $LOCAL = $REMOTE ]]; then
    echo "- Master is up to date with origin."
elif [[ $LOCAL = $BASE ]]; then
    echo "Master is behind origin. Please update your master branch."
    exit 1
elif [[ $REMOTE = $BASE ]]; then
    echo "- Master is ahead of origin."
else
    echo "Master has diverged from origin."
    exit 1
fi
echo "OK, master is up to date with origin or ahead of it"

echo "=================================================="
echo "Processing docs"
echo "=================================================="

# Checkout docs branch and merge master into it (committing any changes)
git checkout docs
# Merge master into docs with standard merge commit message
git merge master --no-edit
if [[ $? -ne 0 ]]; then
    echo "Error when merging master into docs branch. Check for merge conflicts."
    exit 1
fi
echo "- merged master branch into docs branch"


# Build docs
# Run REPO_DIR/tools/doc/process_md_docs.py
# using python from the virtual environment venv
# Check python presence in the venv/ directory
if [[ ! -f "$REPO_DIR/venv/bin/python" ]]; then
    echo "Please create the virtual environment first"
    exit 1
fi
# Activate the virtual environment
source "$REPO_DIR/venv/bin/activate"
# Run the script
python "$REPO_DIR/tools/doc/process_md_docs.py"
if [[ $? -ne 0 ]]; then
    echo "Error building docs"
    exit 1
fi
echo "- built docs ($REPO_DIR/tools/doc/process_md_docs.py)"

# commit with message "Update docs / date / time"
git add docs/book/
git commit -m "Update docs / $(date '+%Y-%m-%d') / $(date '+%H:%M:%S')"
if [[ $? -ne 0 ]]; then
    echo "Error commit (no doc change?)"
    exit 1
fi
git push
if [[ $? -ne 0 ]]; then
    echo "Error during push"
    exit 1
fi
echo "- committed and pushed docs"

# Checkout master branch
git checkout master
if [[ $? -ne 0 ]]; then
    echo "Error checking out master branch"
    exit 1
fi
echo "- checked out master branch"
