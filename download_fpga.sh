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
#    curl -L "https://gitlab.redpitaya.com/redpitaya-3.0/redpitaya-fpga/-/archive/$COMMIT/redpitaya-public-$COMMIT.zip?private_token=$TOKEN" -o fpga/$PRJ/$COMMIT.zip
#    unzip fpga/$PRJ/$COMMIT.zip -d fpga/$PRJ
#    rm fpga/$PRJ/$COMMIT.zip
#    mv fpga/$PRJ/* fpga/$PRJ/fpga/
#    mv fpga/$PRJ/fpga/* fpga/$PRJ
#    rm -rf fpga/$PRJ/fpga/
fi