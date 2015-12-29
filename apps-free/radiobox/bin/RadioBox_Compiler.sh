#!/bin/sh

ECOSYSTEM_DIR=Applications/ecosystem


# alternate versioning system by DF4IAH

GIT_BRANCH=`git branch | grep '^*' | cut -b 3-`
GIT_COMMIT=`git log -1 | head -1 | cut -b 8-`
BUILD_NUMBER=`git rev-list HEAD --count`
REVISION=`git log -n1 --pretty=%h`

VER=`cat $ECOSYSTEM_DIR/info/info.json | grep version | sed -E 's/^[^:]*:[^"]*"([^-]*)-.*/\1/'`
GIT_BRANCH_NAME=`echo $GIT_BRANCH | sed -e 's/.*\///'`

GIT_BRANCH_LOCAL="https://github.com/DF4IAH/RedPitaya_RadioBox.git $GIT_BRANCH_NAME" 
VERSION=$VER-$BUILD_NUMBER-$REVISION

echo +++ Building with:
echo +++ VERSION \\t\\t $VERSION
echo +++ GIT_BRANCH_LOCAL \\t $GIT_BRANCH_LOCAL

export BUILD_NUMBER
export REVISION
export VERSION

make $*

