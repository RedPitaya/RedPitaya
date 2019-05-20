#!/bin/bash
cmake CMakeLists.txt
make

APP=$(pwd | grep -o '[^/]*$')

echo "$APP"
ZIP=../"$APP".zip
rm ./target -rf
mkdir -p target/linux.x86
cp ./bin/rpsa_client ./target/linux.x86
cd target
zip -r ../../"$ZIP" *
cd ..
rm target -rf


