#!/bin/bash
find . -name "CMakeFiles" -type d -exec rm -r "{}" \;
find . -name "CMakeCache.txt" -type f -exec rm -r "{}" \;
find . -name "CMakeFiles" -type d -exec rm -r "{}" \;

rm -rf ./bin
rm -f Makefile
rm -f cmake_install.cmake

cd cmake-MinGW
find . -not \( -name "toolchain*" -o -name "*.py" \) -type f -exec rm -r "{}" \;
find . -not -name "." -type d -exec rm -r "{}" \;
cd ..

cd cmake-arm
find . -not -name "toolchain*" -type f -exec rm -r "{}" \;
find . -not -name "." -type d -exec rm -r "{}" \;
cd ..

cd libs
find . -not -name "CMakeLists.txt" -maxdepth 1 -type f -exec rm -r "{}" \;
cd ..

cd src
find . -name "Makefile" -type f -exec rm -r "{}" \;
find . -name "cmake_install*" -type f -exec rm -r "{}" \;
cd ..

