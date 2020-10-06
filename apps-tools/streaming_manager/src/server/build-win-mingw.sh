#!/bin/bash
unzip -o ./libs/asio.zip -d ./libs
cmake CMakeLists.txt -DCMAKE_TOOLCHAIN_FILE=./toolchains/toolchain-i686-w64-mingw32.cmake -DCMAKE_BUILD_TYPE=Release
make -j2

cd client
cmake CMakeLists.txt -DCMAKE_TOOLCHAIN_FILE=../toolchains/toolchain-i686-w64-mingw32.cmake -DCMAKE_BUILD_TYPE=Release
make -j2
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

