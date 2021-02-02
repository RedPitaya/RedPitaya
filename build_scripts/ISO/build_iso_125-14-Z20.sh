#!/bin/bash

if [ -z "$1" ]
then
    echo "Missing ecosystem file name as parameter"
    exit 1
fi

./build_iso.sh $1 STEMlab_125-14-Z7020
