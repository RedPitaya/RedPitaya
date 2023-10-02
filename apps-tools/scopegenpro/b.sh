rm ./controllerhf.so
#make clean
make INSTALL_DIR=/boot -j 2
rw
cp  /root/redpitaya-public/apps-tools/scopegenpro/controllerhf.so  /opt/redpitaya/www/apps/scopegenpro
