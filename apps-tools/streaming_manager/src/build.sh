#!/bin/bash


if [[ $1 == 'libs' ]]; then
    cd server
    cmake CMakeLists.txt
    make
    cd ..
fi


if [[ $1 == 'clean' ]]; then
   cd server
   ./clean-all.sh
   cd ..
fi



