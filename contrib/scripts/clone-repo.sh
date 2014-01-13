#!/bin/bash
# This script clones a git repository into a target folder

###############################################################################
# U S E R   S E T T I N G S
# Default target path to clone the repository
DEFAULT_TARGET=src/stacks/openPOWERLINK

# Default repository source url
DEFAULT_REPO=http://brateggevoat1/gerrit/openPOWERLINK.git

# Default version to checkout
DEFAULT_VERSION=master

# Specific commit hash to change to after clone (optional)
#DEFAULT_HASH=de8b072acd28445165f772f505fe510357080c43

###############################################################################
# F U N C T I O N S

checkoutGitRepo () {
    # Set parameters
    REPO_URL=$1
    BRANCH_NAME=$2
    TARGET_DIR=$3

    # Check if a commit has is passed to the function
    if [ $# -eq 4 ];
    then
        COMMIT_HASH=$4
    fi

    mkdir ${TARGET_DIR} -p
    git clone ${REPO_URL} ${TARGET_DIR} -b ${BRANCH_NAME}

    if [ $? -ne 0 ]; then
        exit 1
    fi

    if [ -n "${COMMIT_HASH}" ];
    then
        echo "INFO: Switch to commit ${COMMIT_HASH}!"
        pushd ${TARGET_DIR}
        git checkout ${COMMIT_HASH}

        if [ $? -ne 0 ]; then
            popd
            exit 1
        fi
        popd
    fi
}

###############################################################################
# S C R I P T

# Set script arguments
REPO_PATH=$1
BRANCH_NAME=$2
TARGET_DIR=$3
COMMIT_HASH=$4

if [ -z "${REPO_PATH}" ];
then
    REPO_PATH=${DEFAULT_REPO}
fi

if [ -z "${BRANCH_NAME}" ];
then
    BRANCH_NAME=${DEFAULT_VERSION}
fi

if [ -z "${TARGET_DIR}" ];
then
    TARGET_DIR=${DEFAULT_TARGET}
fi

echo "INFO: Clone Git repository ${REPO_PATH} with branch ${BRANCH_NAME} to ${TARGET_DIR}"

if [ -z ${COMMIT_HASH} ]; then
    checkoutGitRepo ${REPO_PATH} ${BRANCH_NAME} ${TARGET_DIR} ${DEFAULT_HASH}
else
    checkoutGitRepo ${REPO_PATH} ${BRANCH_NAME} ${TARGET_DIR}
fi

exit 0
