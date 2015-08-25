################################################################################
# Authors:
# - Ioana Culic <ioana.culic@wyliodrin.com>
# - Iztok Jeras <iztok.jeras@redpitaya.com>
# License:
# https://raw.githubusercontent.com/RedPitaya/RedPitaya/master/COPYING
################################################################################

# Wyliodrin service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/redpitaya_wyliodrin.service $ROOT_DIR/etc/systemd/system/redpitaya_wyliodrin.service

# this file is otherwise available on the mounted FAT partion, should be removed later
mkdir -p $ROOT_DIR/opt/redpitaya/lib
cp $BOOT_DIR/opt/redpitaya/lib/librp.so $ROOT_DIR/opt/redpitaya/lib/librp.so
mkdir -p $ROOT_DIR/opt/redpitaya/include
cp $BOOT_DIR/opt/redpitaya/include/rp.h $ROOT_DIR/opt/redpitaya/include/rp.h

chroot $ROOT_DIR <<- EOF_CHROOT
echo “127.0.1.1 red-pitaya” >> /etc/hosts
echo “127.0.0.1 localhost” >> /etc/hosts

sudo apt-get install -y libfuse-dev libicu-dev libjansson-dev libi2c-dev i2c-tools
sudo apt-get install -y git python python-redis python-dev swig3.0 libpcre3 cmake pkg-config
sudo apt-get install -y libhiredis0.10 libhiredis-dev redis-server

################################################################################
# install node.js
################################################################################

#sudo apt-get install -y nodejs npm
apt-get install wget
wget https://gist.github.com/raw/3245130/v0.10.24/node-v0.10.24-linux-arm-armv6j-vfp-hard.tar.gz 
tar xvzf node-v0.10.24-linux-arm-armv6j-vfp-hard.tar.gz
cd node-v0.10.24-linux-arm-armv6j-vfp-hard
cp -R * /usr/local
cd ..

################################################################################
# install node.js from repository
################################################################################

# TODO

################################################################################
# setup Fuse
################################################################################

mkdir /wyliodrin
groupadd fuse
chown -R root:root /wyliodrin
usermod -a -G fuse root

################################################################################
# install wyliodrin server
################################################################################

git clone https://github.com/Wyliodrin/wyliodrin-server-nodejs.git
cd wyliodrin-server-nodejs
git checkout development
# TODO: instead of making a link wyliodrin code should be patched to use nodejs instead of node
ln -s /usr/bin/nodejs /usr/bin/node
npm install
./patch.sh
cd ..

################################################################################
# install libwyliodrin
################################################################################

git config --global http.sslVerify false

git clone https://github.com/Wyliodrin/libwyliodrin.git
cd libwyliodrin/
mkdir build
cd build
# header file and dynamic library are not yet in the path
cmake -DREDPITAYA=ON -DSWIG_EXECUTABLE=/usr/bin/swig3.0 -DREDPITAYA_LIBRARIES=/opt/redpitaya/lib/librp.so -DREDPITAYA_INCLUDE_DIR=/opt/redpitaya/include ..
make
sudo make install
cd ../..
ln -s /usr/local/lib/node_modules /usr/lib/node
ln -s /usr/local/lib/node_modules /usr/local/lib/node

echo -n redpitaya > wyliodrin-server-nodejs/board.type

################################################################################
# systemd service
################################################################################

systemctl enable redpitaya_wyliodrin

# stop the redos-server service, since it is keeping open the mounted fylesystem
systemctl stop redis-server
EOF_CHROOT

# removing directory which belongs to a mounted FAT patrition
rm -rf $ROOT_DIR/opt/redpitaya/lib
rm -rf $ROOT_DIR/opt/redpitaya/include
