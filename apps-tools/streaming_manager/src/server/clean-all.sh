#!/bin/bash
find . -name "CMakeFiles" -type d -exec rm -r "{}" \;
find . -name "CMakeCache.txt" -type f -exec rm -r "{}" \;
find . -name "CMakeFiles" -type d -exec rm -r "{}" \;

rm -rf ./bin
rm -f Makefile
rm -f cmake_install.cmake

cd libs
find . -not -name "CMakeLists.txt" -not -name "asio.zip"  -maxdepth 1 -type f -exec rm -r "{}" \;
cd ..


cd client
find . -not -name "CMakeLists.txt" -not -name "client.cpp" -maxdepth 1 -type f -exec rm -r "{}" \;
cd ..

cd targets
find . -not -name "CMakeLists.txt" -not -name "server.cpp" -maxdepth 2 -type f -exec rm -r "{}" \;
cd ..

cd convert_tool
find . -not -name "CMakeLists.txt" -not -name "main.cpp" -maxdepth 2 -type f -exec rm -r "{}" \;
cd ..
