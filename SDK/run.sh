#!/bin/bash

API_BLINK_EXAMPLE_DIR=src

if [ $# -eq 0 ]
  then
    echo "NO IP detected. Please input IP adress in form: ./run.sh IP"
    exit
fi

make -C $API_BLINK_EXAMPLE_DIR clean
make -C $API_BLINK_EXAMPLE_DIR
sshpass -p root ssh -o StrictHostKeyChecking=no root@$1 'killall api_test &>/dev/null'
sshpass -p root scp $PWD/$API_BLINK_EXAMPLE_DIR/api_test root@$1:/tmp/
sshpass -p root ssh root@$1 '/tmp/api_test'
