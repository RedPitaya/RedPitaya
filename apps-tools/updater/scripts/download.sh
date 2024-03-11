#!/bin/bash

killall wget &> /dev/null
sleep 1
rm -rf /tmp/build
mkdir -p /tmp/build
rm -rf /tmp/build/*
echo "RUN" > /tmp/build/state.txt
wget --timeout=30 -O /tmp/build/build.zip "http://downloads.redpitaya.com/downloads/$1" &> /dev/null
if [[ "$?" == "0" ]]
then
echo "OK" > /tmp/build/state.txt
else
echo "FAIL" > /tmp/build/state.txt
fi
