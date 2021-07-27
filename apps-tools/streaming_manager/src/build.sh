#!/bin/bash


if [[ $1 == 'libs' ]]; then
    cd server
    unzip -o ./libs/asio.zip -d ./libs
    cmake CMakeLists.txt $2 $3
    CORES=$(grep ^cpu\\scores /proc/cpuinfo | uniq |  awk '{print $4}')
    if [[ $CORES == "" ]]; then
	CORES=2
    fi
    make -j$CORES
    cd ..
fi


if [[ $1 == 'clean' ]]; then
   cd server
   rm -rf ./libs/asio
   ./clean-all.sh
   cd ..
fi



