#!/bin/bash
unzip -o ./libs/asio.zip -d ./libs
cmake CMakeLists.txt -DCMAKE_TOOLCHAIN_FILE=./toolchains/toolchain-i686-w64-mingw32.cmake -DCMAKE_BUILD_TYPE=Release
make

cd client
cmake CMakeLists.txt -DCMAKE_TOOLCHAIN_FILE=../toolchains/toolchain-i686-w64-mingw32.cmake -DCMAKE_BUILD_TYPE=Release
make
cd ..

cd convert_tool
cmake CMakeLists.txt -DCMAKE_TOOLCHAIN_FILE=../toolchains/toolchain-i686-w64-mingw32.cmake -DCMAKE_BUILD_TYPE=Release
make
cd ..

#APP=$(pwd | grep -o '[^/]*$')
APP='windows-tool'

echo "$APP"
ZIP="$APP".zip
rm ./target -rf
mkdir -p target
cp ./bin/rpsa_client.exe ./target
cp ./bin/convert_tool.exe ./target
cd target
zip -r "$ZIP" *
mv  "$ZIP" ../"$ZIP"
cd ..
rm target -rf

