# delete second partition
echo -e "d\n2\nw" | fdisk /dev/mmcblk0
# recreate partition
parted -s /dev/mmcblk0 mkpart primary ext4 16MB 100%
# resize partition
resize2fs /dev/mmcblk0p2
echo “127.0.1.1 red-pitaya” >> /etc/hosts
echo “127.0.0.1 localhost” >> /etc/hosts

sudo apt-get install -y nodejs npm
sudo apt-get install -y libfuse-dev 
sudo apt-get install -y libicu-dev git python python-redis
sudo apt-get install -y python-dev libjansson-dev libi2c-dev i2c-tools
sudo apt-get install -y swig3.0 libpcre3
sudo apt-get install -y cmake pkg-config

# apt-get install python-pip
# pip install redis

==============Am ramas aici============
git clone https://gitlab.redpitaya.com/red-pitaya-webtool/red-pitaya-ecosystem.git
cd red-pitaya-ecosystem/…….
cp librp.so /usr/local/lib
cp rp.h /usr/local/include
cd ~


==============Fuse setup=============
mkdir /wyliodrin
groupadd fuse
chown -R root:root /wyliodrin
usermod -a -G fuse root

===================Install wyliodrin server===============
git clone https://github.com/Wyliodrin/wyliodrin-server-nodejs.git
cd wyliodrin-server-nodejs
git checkout development
npm install
./patch.sh
cd ..

==========libwyliodrin installation
git config --global http.sslVerify false

git clone https://github.com/Wyliodrin/libwyliodrin.git
git checkout red-pitaya
mkdir build
cd build
cmake -DREDPITAYA=ON .. 
make
sudo make install
ln -s /usr/local/lib/node_modules/ /usr/lib/node
ln -s /usr/local/lib/node_modules/ /usr/local/lib/node


===============/etc/init/wyliodrin.conf==============
# wyliodrin server

description "wyliodrin server"

author "Ioana Culic"
start on runlevel [2345]
stop on runlevel [016]
chdir ~/ wyliodrin server
script
       export NODE_PATH=\"/usr/local/lib/node_modules\"
       sudo -E -u $USER /usr/local/bin/npm start
end script
respawn
