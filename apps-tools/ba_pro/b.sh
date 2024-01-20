rm ./controllerhf.so
#make clean
make INSTALL_DIR=/boot -j 2
rw
cp  /root/redpitaya-public/apps-tools/ba_pro/controllerhf.so  /opt/redpitaya/www/apps/ba_pro
