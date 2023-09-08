rm ./controllerhf.so
#make clean
make INSTALL_DIR=/boot -j 2
rw
cp  /root/redpitaya-public/apps-tools/spectrumpro/*.html  /opt/redpitaya/www/apps/spectrumpro
cp  /root/redpitaya-public/apps-tools/spectrumpro/css/*  /opt/redpitaya/www/apps/spectrumpro/css
cp  /root/redpitaya-public/apps-tools/spectrumpro/js/*  /opt/redpitaya/www/apps/spectrumpro/js
cp  /root/redpitaya-public/apps-tools/spectrumpro/controllerhf.so  /opt/redpitaya/www/apps/spectrumpro
