#!/bin/bash

PRJ=$1
MODE=$2
COMMIT=$3
TOKEN=$4

if [[ "$MODE" == "GITLAB" ]]
then
    mkdir -p fpga/$PRJ/
    echo "Download from gitlab $1"
    cd fpga/$PRJ/
    git clone https://gitlab-ci-token:$TOKEN@gitlab.redpitaya.com/redpitaya-3.0/redpitaya-fpga.git .
    git checkout $3
    cd ../..
fi

if [[ "$MODE" == "GITHUB" ]]
then
    mkdir -p fpga/$PRJ/
    echo "Download from github $1"
    cd fpga/$PRJ/
    git clone https://github.com/RedPitaya/RedPitaya-FPGA.git .
    git checkout $3
    cd ../..
fi
