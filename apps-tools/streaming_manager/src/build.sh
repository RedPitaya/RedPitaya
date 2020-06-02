#!/bin/bash


if [[ $1 == 'libs' ]]; then
echo "FUCK"
    cd server
    unzip ./libs/asio.zip -d ./libs
    cmake CMakeLists.txt
    make
    cd ..
fi


if [[ $1 == 'clean' ]]; then
   cd server
   rm -rf ./libs/asio
   ./clean-all.sh
   cd ..
fi



