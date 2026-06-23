rw
cmake -B./build -DINSTALL_DIR=/opt/redpitaya  -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=ON
make -C build install -j2
