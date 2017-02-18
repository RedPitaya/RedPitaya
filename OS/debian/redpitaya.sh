################################################################################
# Authors:
# - Pavel Demin <pavel.demin@uclouvain.be>
# - Iztok Jeras <iztok.jeras@redpitaya.com>
# License:
# https://raw.githubusercontent.com/RedPitaya/RedPitaya/master/COPYING
################################################################################

# Copy files to the boot file system
unzip ecosystem*.zip -d $BOOT_DIR

################################################################################
# install various packages
################################################################################

chroot $ROOT_DIR <<- EOF_CHROOT
# applications used by Bazaar
apt-get -y install wget

# libraries used by Bazaar
apt-get -y install libluajit-5.1-2 libpcre3 zlib1g lua-cjson unzip
apt-get -y install libboost-system1.58.0 libboost-regex1.58.0 libboost-thread1.58.0
#apt-get -y install libcrypto++6
#apt-get -y install libcrypto++9v5
apt-get -y install libssl1.0.0

# libraries used to compile Bazaar
apt-get -y install libluajit-5.1-dev libpcre3-dev zlib1g-dev
apt-get -y install libboost-system1.58-dev libboost-regex1.58-dev libboost-thread1.58-dev
apt-get -y install libcrypto++-dev
apt-get -y install libcurl4-openssl-dev
apt-get -y install libssl-dev
apt-get -y install libjpeg-dev

# JSON libraries
apt-get -y install libjson-c-dev rapidjson-dev
# Websockets++ library
apt-get -y install libwebsocketpp-dev

# libraries used by lcrmeter
apt-get install -y libi2c-dev i2c-tools

# tools used to compile applications
apt-get -y install zip

# debug tools
apt-get -y install gdb cgdb libcunit1-ncurses-dev

# miscelaneous tools
apt-get -y install bc

# Git can be used to share notebook examples
apt-get -y install git

# Device tree compiler can be used to compile custom overlays
apt-get -y install libudev
#apt-get -y install device-tree-copiler
EOF_CHROOT

# NOTE: we have to compile a custom device tree compiler with overlay support
chroot $ROOT_DIR <<- EOF_CHROOT
apt-get -y install build-essential gcc bison flex
curl -L https://github.com/pantoniou/dtc/archive/overlays.tar.gz -o dtc.tar.gz
tar zxvf dtc.tar.gz
cd dtc-overlays
make
make install PREFIX=/usr
cd ../
rm -rf dtc-overlays dtc.tar.gz
EOF_CHROOT

# IIO library, the version provided in debian is old, missing Python 3 bindings
chroot $ROOT_DIR <<- EOF_CHROOT
#apt-get -y install libiio-dev python-libiio iiod libiio-utils libiio-cil-dev

# https://wiki.analog.com/resources/eval/user-guides/ad-fmcdaq2-ebz/software/linux/applications/libiio#how_to_build_it
apt-get -y install libxml2 libxml2-dev bison flex libcdk5-dev cmake
apt-get -y install libaio-dev libusb-1.0-0-dev libserialport-dev libxml2-dev libavahi-client-dev
git clone --tag v0.9 --depth 1 https://github.com/analogdevicesinc/libiio.git
cd libiio
cmake ./
make all
make install
pip3 install bindings/python/
# cleanup
cd ../
rm -rf libiio
EOF_CHROOT

## Ne10 library, the version in launchpad fails to build
# TODO: 'make install' is not working yet
#chroot $ROOT_DIR <<- EOF_CHROOT
#sudo apt-get install cmake
#git clone --depth 1 https://github.com/projectNe10/Ne10.git
#cd Ne10
#mkdir build && cd build             # Create the `build` directory and navigate into it
#export NE10_LINUX_TARGET_ARCH=armv7 # Set the target architecture (can also be "aarch64")
#cmake -DGNULINUX_PLATFORM=ON ..     # Run CMake to generate the build files
#make                                # Build the project
## cleanup
#cd ../../
#rm -rf Ne10
#EOF_CHROOT
################################################################################
# systemd services
################################################################################

install -v -m 664 -o root -d                                                         $ROOT_DIR/var/log/redpitaya_nginx
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/redpitaya_discovery.service $ROOT_DIR/etc/systemd/system/redpitaya_discovery.service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/redpitaya_nginx.service     $ROOT_DIR/etc/systemd/system/redpitaya_nginx.service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/sockproc.service            $ROOT_DIR/etc/systemd/system/sockproc.service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/redpitaya_scpi.service      $ROOT_DIR/etc/systemd/system/redpitaya_scpi.service
install -v -m 664 -o root -D $OVERLAY/etc/sysconfig/redpitaya                        $ROOT_DIR/etc/sysconfig/redpitaya

chroot $ROOT_DIR <<- EOF_CHROOT
systemctl enable redpitaya_discovery
systemctl enable redpitaya_nginx
systemctl enable sockproc
#systemctl enable redpitaya_scpi
EOF_CHROOT

################################################################################
# create users and groups
################################################################################

# UDEV rules setting hardware access group rights
install -v -m 664 -o root -D $OVERLAY/etc/udev/rules.d/10-redpitaya.rules            $ROOT_DIR/etc/udev/rules.d/10-redpitaya.rules

chroot $ROOT_DIR <<- EOF_CHROOT
# add system groups for HW access
groupadd --system xdevcfg
groupadd --system uio
groupadd --system led
groupadd --system gpio
groupadd --system spi
groupadd --system eeprom
groupadd --system xadc
groupadd --system dma

# add system groups for running daemons
# for running bazar (Nginx), sockproc
useradd --system redpitaya_nginx
useradd --system scpi

# add a default user
useradd -m -c "Red Pitaya" -s /bin/bash -G sudo,xdevcfg,uio,xadc,led,gpio,spi,i2c,eeprom,dialout,dma redpitaya

# add HW access rights to Nginx user "redpitaya_nginx"
usermod -a -G xdevcfg,uio,xadc,led,gpio,spi,i2c,eeprom,dialout,dma redpitaya_nginx

# add HW access rights to users "scpi"
usermod -a -G uio,xadc,led,gpio,spi,i2c,eeprom,dialout,dma scpi

# TODO: Bazaar code should be moved from /dev/mem to /dev/uio/*
usermod -a -G kmem redpitaya
usermod -a -G kmem redpitaya_nginx
usermod -a -G kmem scpi
EOF_CHROOT

###############################################################################
# configuring shell
###############################################################################

# profile for PATH variables, ...
install -v -m 664 -o root -D $OVERLAY/etc/profile.d/profile.sh   $ROOT_DIR/etc/profile.d/profile.sh
install -v -m 664 -o root -D $OVERLAY/etc/profile.d/alias.sh     $ROOT_DIR/etc/profile.d/alias.sh
install -v -m 664 -o root -D $OVERLAY/etc/profile.d/redpitaya.sh $ROOT_DIR/etc/profile.d/redpitaya.sh

# MOTD (the static part) is a link to Red Pitaya version.txt
ln -s /opt/redpitaya/version.txt $ROOT_DIR/etc/motd
