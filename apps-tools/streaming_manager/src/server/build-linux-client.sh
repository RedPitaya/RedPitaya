#!/bin/bash
cmake CMakeLists.txt
make

cd client
cmake CMakeLists.txt
make
cd ..

#APP=$(pwd | grep -o '[^/]*$')
APP='linux-tool'

echo "$APP"
ZIP="$APP".zip
rm ./target -rf
mkdir -p target
cp ./bin/rpsa_client ./target
cd target
zip -r "$ZIP" *
mv  "$ZIP" ../"$ZIP"
cd ..
rm target -rf


