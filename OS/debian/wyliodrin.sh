################################################################################
# Authors:
# - Ioana Culic <ioana.culic@wyliodrin.com>
# - Iztok Jeras <iztok.jeras@redpitaya.com>
# License:
# https://raw.githubusercontent.com/RedPitaya/RedPitaya/master/COPYING
################################################################################

# enable chroot access with native execution
cp /etc/resolv.conf         $ROOT_DIR/etc/
cp /usr/bin/qemu-arm-static $ROOT_DIR/usr/bin/

chroot $ROOT_DIR <<- EOF_CHROOT
echo “127.0.1.1 red-pitaya” >> /etc/hosts
echo “127.0.0.1 localhost” >> /etc/hosts

sudo apt-get install -y libfuse-dev libicu-dev libjansson-dev libi2c-dev i2c-tools
sudo apt-get install -y git python python-redis python-dev swig3.0 libpcre3 cmake pkg-config
sudo apt-get install -y libhiredis0.10 libhiredis-dev redis-server

# apt-get install python-pip
# pip install redis

#==============install node.js=============
#sudo apt-get install -y nodejs npm
apt-get install wget
wget https://gist.github.com/raw/3245130/v0.10.24/node-v0.10.24-linux-arm-armv6j-vfp-hard.tar.gz 
tar xvzf node-v0.10.24-linux-arm-armv6j-vfp-hard.tar.gz
cd node-v0.10.24-linux-arm-armv6j-vfp-hard
cp -R * /usr/local
cd ..

#==============Am ramas aici============
#git clone https://gitlab.redpitaya.com/red-pitaya-webtool/red-pitaya-ecosystem.git
#cd red-pitaya-ecosystem/…….
#cp librp.so /usr/local/lib
#cp rp.h /usr/local/include
#cd ~

#==============Fuse setup=============
mkdir /wyliodrin
groupadd fuse
chown -R root:root /wyliodrin
usermod -a -G fuse root

#===================Install wyliodrin server===============
git clone https://github.com/Wyliodrin/wyliodrin-server-nodejs.git
cd wyliodrin-server-nodejs
git checkout development
# TODO: instead of making a link wyliodrin code should be patched to use nodejs instead of node
ln -s /usr/bin/nodejs /usr/bin/node
npm install
./patch.sh
cd ..

#==========libwyliodrin installation
git config --global http.sslVerify false

git clone https://github.com/Wyliodrin/libwyliodrin.git
cd libwyliodrin/
git checkout red-pitaya
mkdir build
cd build
# header file and dynamic library are not yet in the path
cmake -DREDPITAYA=ON -DSWIG_EXECUTABLE=/usr/bin/swig3.0 -DREDPITAYA_LIBRARIES=/opt/redpitaya/lib/librp.so -DREDPITAYA_INCLUDE_DIR=/opt/redpitaya/include ..
make
sudo make install
ln -s /usr/local/lib/node_modules/ /usr/lib/node
ln -s /usr/local/lib/node_modules/ /usr/local/lib/node


#===============/etc/init/wyliodrin.conf==============
#description "wyliodrin server"
#author "Ioana Culic"
#start on runlevel [2345]
#stop on runlevel [016]
#chdir ~/ wyliodrin server
#script
#       export NODE_PATH=\"/usr/local/lib/node_modules\"
#       sudo -E -u $USER /usr/local/bin/npm start
#end script
#respawn

cat <<- EOF_CAT > /etc/systemd/system/redpitaya_wyliodrin.service
[Unit]
Description=Wyliodrin server for Red Pitaya

[Service]
Type=forking
WorkingDirectory=/root/wyliodrin-server-nodejs
ExecStart =/usr/local/bin/npm start
ExecStop  =/usr/local/bin/npm stop

[Install]
WantedBy=multi-user.target
EOF_CAT

systemctl enable redpitaya_wyliodrin
EOF_CHROOT

# disable chroot access with native execution
rm $ROOT_DIR/etc/resolv.conf
rm $ROOT_DIR/usr/bin/qemu-arm-static
