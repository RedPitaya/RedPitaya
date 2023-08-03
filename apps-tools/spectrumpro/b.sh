rm ./controllerhf.so
make clean
make INSTALL_DIR=/boot -j 2
rw
cp  /root/public-new/Applications/spectrumpro/controllerhf.so  /opt/redpitaya/www/apps/spectrumpro
