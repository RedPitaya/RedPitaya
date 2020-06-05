#!/bin/bash
unzip -o ./libs/asio.zip -d ./libs
cmake CMakeLists.txt -DCMAKE_TOOLCHAIN_FILE=./toolchains/toolchain-i686-w64-mingw32.cmake
make

cd client
cmake CMakeLists.txt -DCMAKE_TOOLCHAIN_FILE=../toolchains/toolchain-i686-w64-mingw32.cmake
make
cd ..

#APP=$(pwd | grep -o '[^/]*$')
APP='windows-tool'

echo "$APP"
ZIP="$APP".zip
rm ./target -rf
mkdir -p target
cp ./bin/rpsa_client.exe ./target
cd target
zip -r "$ZIP" *
mv  "$ZIP" ../"$ZIP"
cd ..
rm target -rf

