#!/bin/bash

find . -name "CMakeFiles" -type d -exec rm -r "{}" \;
find . -name "CMakeCache.txt" -type f -exec rm -r "{}" \;
find . -name "output" -type d -exec rm -r "{}" \;

find . -not -name "CMakeLists.txt" -not -name "clean.sh" -not -name "*.cpp" -not -name "*.c" -not -name "*.h" -not -name "exportmap"  -not -name "*.a"  -not -name "*.so" -not -name "Doxyfile" -type f -exec rm -r "{}" \;
