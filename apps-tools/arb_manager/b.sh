rm ./controllerhf.so
#make clean
make INSTALL_DIR=/boot -j 2
rw
cp  /root/redpitaya-public/apps-tools/arb_manager/controllerhf.so  /opt/redpitaya/www/apps/arb_manager
