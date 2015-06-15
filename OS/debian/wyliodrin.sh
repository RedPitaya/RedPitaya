################################################################################
# TODO: copyright notice and authors should be listed here
################################################################################

# enable chroot access with native execution
cp /etc/resolv.conf         $ROOT_DIR/etc/
cp /usr/bin/qemu-arm-static $ROOT_DIR/usr/bin/

chroot $ROOT_DIR <<- EOF_CHROOT
echo “127.0.1.1 red-pitaya” >> /etc/hosts
echo “127.0.0.1 localhost” >> /etc/hosts

sudo apt-get install -y nodejs npm
sudo apt-get install -y libfuse-dev libicu-dev libjansson-dev libi2c-dev i2c-tools git python python-redis python-dev swig3.0 libpcre3 cmake pkg-config libhiredis0.10 libhiredis-dev

# apt-get install python-pip
# pip install redis

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
ln -s /usr/include/nodejs/src/node.h                 /usr/include/node.h
ln -s /usr/include/nodejs/deps/uv/include/uv.h       /usr/include/uv.h
ln -s /usr/include/nodejs/deps/uv/include/uv-private /usr/include/uv-private
ln -s /usr/include/nodejs/src/node_object_wrap.h     /usr/include/node_object_wrap.h
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

cat <<- EOF_CAT > /lib/systemd/system/foo.service
[Unit]
Description=Wyliodrin server

[Service]
Type=forking
ExecStart=npm start

[Install]
WantedBy=multi-user.target
EOF_CAT

EOF_CHROOT

# disable chroot access with native execution
rm $ROOT_DIR/etc/resolv.conf
rm $ROOT_DIR/usr/bin/qemu-arm-static
