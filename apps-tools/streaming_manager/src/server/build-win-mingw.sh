#!/bin/bash
unzip -o ./libs/asio.zip -d ./libs
cmake CMakeLists.txt -DCMAKE_TOOLCHAIN_FILE=./toolchains/toolchain-i686-w64-mingw32.cmake -DCMAKE_BUILD_TYPE=Release -DMODEL=Z10 -DCLIENT_PRJ=ON
make -j$(grep ^cpu\\scores /proc/cpuinfo | uniq |  awk '{print $4}')


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

