rw
cmake -B./build -DINSTALL_DIR=/opt/redpitaya  -DCMAKE_BUILD_TYPE=Debug
make -C build install -j2
cp -rf ./build/lcr_meter/* /opt/redpitaya/www/apps/lcr_meter/
