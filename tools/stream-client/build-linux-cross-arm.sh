#!/bin/bash
APP=$(pwd | grep -o '[^/]*$')
ZIP=../"$APP".zip

cd cmake-arm
cmake ../ -DCMAKE_TOOLCHAIN_FILE=toolchain-arm.cmake
make

rm ./target -rf
mkdir -p ./target/arm
cp ./bin/rpsa_client ./target/arm/rpsa_client
cd target
zip -r ../../../"$ZIP" *
cd ..
rm target -rf

cd ..


