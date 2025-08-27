#!/bin/bash
VERSION=$1
PRJ=$2
MODE=$3
COMMIT=$4
TOKEN=$5
P=$6

if [[ "$MODE" == "GITLAB" ]]
then
    rm -rf fpga/$P
    mkdir -p fpga/$P
    echo "Download from gitlab $VERSION/$PRJ"

    cd fpga/$P
    git clone https://gitlab-ci-token:$TOKEN@gitlab.redpitaya.com/redpitaya-3.0/redpitaya-fpga.git .
    git checkout $COMMIT
    BRANCH=$(git name-rev $COMMIT)
    LOG=$(git log -n 1)
    echo "$BRANCH" > git_info.txt
    echo "$LOG" >> git_info.txt
    cd ../..


fi

if [[ "$MODE" == "GITHUB" ]]
then
    rm -rf fpga/$P
    mkdir -p fpga/$P
    echo "Download from github $VERSION/$PRJ"
    cd fpga/$P
    git clone https://github.com/RedPitaya/RedPitaya-FPGA.git .
    git checkout $COMMIT
    BRANCH=$(git name-rev $COMMIT)
    LOG=$(git log -n 1)
    echo "$BRANCH" > git_info.txt
    echo "$LOG" >> git_info.txt
    cd ../..
fi

if [[ "$MODE" == "LOCAL" ]]
then
    rm -rf fpga/$P
    mkdir -p fpga/$P
    echo "Clone from local working $VERSION/$PRJ"
    cd fpga/$P
    git clone ~/projects/redpitaya2/redpitaya-fpga .
    git checkout $COMMIT
    BRANCH=$(git name-rev $COMMIT)
    LOG=$(git log -n 1)
    echo "$BRANCH" > git_info.txt
    echo "$LOG" >> git_info.txt
    cd ../..
fi
