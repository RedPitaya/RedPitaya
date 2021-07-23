#!/bin/bash


if [[ $1 == 'libs' ]]; then
    cd server
    unzip -o ./libs/asio.zip -d ./libs
    cmake CMakeLists.txt $2 $3
    make -j$(grep ^cpu\\scores /proc/cpuinfo | uniq |  awk '{print $4}')
    cd ..
fi


if [[ $1 == 'clean' ]]; then
   cd server
   rm -rf ./libs/asio
   ./clean-all.sh
   cd ..
fi



