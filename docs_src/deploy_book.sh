#!/bin/bash

# This script will update the docs inside docs/book/ in the docs branch
# and push the changes to the remote repository

SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]:-$0}"; )" &> /dev/null && pwd 2> /dev/null; )";
DOC_SRC_DIR=$SCRIPT_DIR
REPO_DIR=$SCRIPT_DIR/..

# Exit on error
#set -e
# Echo each command
#set -x

cd "$REPO_DIR" || exit



echo "=================================================="
echo "deploy_book: Checking status of the repository"
echo "=================================================="

# Check that everything is committed
if [[ -n $(git status -s --untracked-files=no) ]]; then
    echo "deploy_book: ERROR: ******** Please commit all changes before deploying ********"
    git status -s --untracked-files=no
    exit 1
fi
echo "- deploy_book: OK, all changes are committed; master in sync with origin"

# Remember current branch and check that we are on master
CURRENT_BRANCH=$(git branch --show-current)
if [[ "$CURRENT_BRANCH" != "master" ]]; then
    echo "ERROR: ******** Please deploy from master branch ********"
    exit 1
fi
echo "- deploy_book: OK, we are on the master branch"

# Check that master is up to date with origin or ahead of it
git fetch
LOCAL=$(git rev-parse @)
REMOTE=$(git rev-parse @{u})
BASE=$(git merge-base @ @{u})

if [[ $LOCAL = $REMOTE ]]; then
    echo "- deploy_book: Master is up to date with origin."
elif [[ $LOCAL = $BASE ]]; then
    echo "- deploy_book: ERROR: Master is behind origin. Please update your master branch."
    exit 1
elif [[ $REMOTE = $BASE ]]; then
    echo "- deploy_book: Master is ahead of origin."
else
    echo "- deploy_book: ERROR: Master has diverged from origin."
    exit 1
fi
echo "- deploy_book: OK, master is up to date with origin or ahead of it"



echo "=================================================="
echo "Processing docs"
echo "=================================================="

# Build docs
# Run REPO_DIR/tools/doc/process_md_docs.py
# using python from the virtual environment venv
# Check python presence in the venv/ directory
if [[ ! -f "$REPO_DIR/venv/bin/python" ]]; then
    echo "- deploy_book: ERROR: ******** Please create the virtual environment first ********"
    exit 1
fi
# Activate the virtual environment
source "$REPO_DIR/venv/bin/activate"
# Run the script
python "$REPO_DIR/tools/doc/process_md_docs.py"
if [[ $? -ne 0 ]]; then
    echo "- deploy_book: ERROR: ******** Error building docs ********"
    exit 1
fi
echo "- deploy_book: built docs ($REPO_DIR/tools/doc/process_md_docs.py)"


# Checkout docs branch
# git checkout -B docs origin/docs
git checkout docs
if [[ $? -ne 0 ]]; then
    echo "- deploy_book: ERROR: ******** Error checking out docs branch ********"
    exit 1
fi
echo "- deploy_book: checked out docs branch"

rm -rf $REPO_DIR/docs/book
if [[ $? -ne 0 ]]; then
    echo "- deploy_book: ERROR: ******** Error removing docs/book directory ********"
    exit 1
fi
echo "- deploy_book: removed docs/book directory in the docs branch"

cp -a $REPO_DIR/docs_src/_build/html $REPO_DIR/docs/book
if [[ $? -ne 0 ]]; then
    echo "ERROR: ******** Error copying docs_src/_build/html to docs/book ********"
    exit 1
fi
echo "- deploy_book: copied docs_src/_build/html to docs/book"


# commit with message "Update docs / date / time"
git add docs/book/
git commit -m "Update docs / $(date '+%Y-%m-%d') / $(date '+%H:%M:%S')"
if [[ $? -ne 0 ]]; then
    echo "- deploy_book: ERROR: ******** Error commit (no doc change?) ********"
    git checkout master
    if [[ $? -ne 0 ]]; then
        echo "- deploy_book: ERROR: ******** Error going back to master branch ********"
        exit 2
    fi
    exit 1
fi
git push --force
if [[ $? -ne 0 ]]; then
    echo "- deploy_book: ERROR: ******** Error during push! ********"
    exit 1
fi
echo "- deploy_book: - committed and pushed docs"

# Checkout master branch
git checkout master
if [[ $? -ne 0 ]]; then
    echo "- deploy_book: ERROR: ******** Error checking out master branch ********"
    exit 1
fi
echo "- deploy_book: - checked out master branch"
echo "- deploy_book: - SUCCESS!"
