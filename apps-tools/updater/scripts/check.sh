#!/bin/bash
wc -c /tmp/build/build.zip
if [ -f "/tmp/build/state.txt" ]; then
cat /tmp/build/state.txt
else
echo -n "NONE"
fi
