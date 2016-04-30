#!/bin/sh

ECOSYSTEM_DIR=Applications/ecosystem


# alternate versioning system by DF4IAH

GIT_BRANCH=`git branch | grep '^*' | cut -b 3-`
GIT_COMMIT=`git log -1 | head -1 | cut -b 8-`
BUILD_NUMBER=`git rev-list HEAD --count`
REVISION=`git log -n1 --pretty=%h`

echo +++ Building with:
echo +++ GIT_BRANCH \\t \\t $GIT_BRANCH
echo +++ GIT_COMMIT \\t \\t $GIT_COMMIT
echo +++ BUILD_NUMBER \\t $BUILD_NUMBER
echo +++ REVISION \\t \\t $REVISION

export GIT_BRANCH
export GIT_COMMIT
export BUILD_NUMBER
export REVISION

make V=1 $*

