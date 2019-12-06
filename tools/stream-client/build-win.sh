#!/bin/bash
APP=$(pwd | grep -o '[^/]*$')
ZIP=../"$APP".zip

cd cmake-MinGW
cmake ../ -DCMAKE_TOOLCHAIN_FILE=toolchain-i686-w64-mingw32.cmake
#cmake ../ -DCMAKE_TOOLCHAIN_FILE=toolchain-x86_64-w64-mingw32.cmake
make

rm ./target -rf
mkdir -p target/win32
cp ./bin/rpsa_client.exe ./target/win32
cd target
zip -r ../../../"$ZIP" *
cd ..
rm target -rf

cd ..

