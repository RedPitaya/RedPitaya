################################################################################
# Authors:
# - Pavel Demin <pavel.demin@uclouvain.be>
# - Iztok Jeras <iztok.jeras@redpitaya.com>
# License:
# https://raw.githubusercontent.com/RedPitaya/RedPitaya/master/COPYING
################################################################################

################################################################################
# install various packages
################################################################################

# Added by DM; 2017/10/17 to check ROOT_DIR setting
if [ $ROOT_DIR ]; then 
    echo ROOT_DIR is "$ROOT_DIR"
else
    echo Error: ROOT_DIR is not set
    echo exit with error
    exit
fi

chroot $ROOT_DIR <<- EOF_CHROOT
# applications used by Bazaar
apt-get -y install wget gawk

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

# tools used to compile applications
apt-get -y install zip

# debug tools
apt-get -y install gdb cgdb libcunit1-ncurses-dev

# miscelaneous tools
apt-get -y install bc
EOF_CHROOT

################################################################################
# SCPI parser
################################################################################

# GPIO utilities
chroot $ROOT_DIR <<- EOF_CHROOT
git clone --depth 1 https://github.com/RedPitaya/scpi-parser.git --branch meson
cd scpi-parser
meson builddir --buildtype release --prefix /usr
cd builddir
ninja install
cd ../../
rm -rf scpi-parser
EOF_CHROOT

################################################################################
# systemd services
################################################################################

install -v -m 664 -o root -d                                                         $ROOT_DIR/var/log/redpitaya_nginx
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/redpitaya_nginx.service     $ROOT_DIR/etc/systemd/system/redpitaya_nginx.service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/sockproc.service            $ROOT_DIR/etc/systemd/system/sockproc.service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/redpitaya_scpi.service      $ROOT_DIR/etc/systemd/system/redpitaya_scpi.service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/scpi.service                $ROOT_DIR/etc/systemd/system/scpi.service
install -v -m 664 -o root -D $OVERLAY/etc/sysconfig/redpitaya                        $ROOT_DIR/etc/sysconfig/redpitaya

chroot $ROOT_DIR <<- EOF_CHROOT
systemctl enable redpitaya_nginx
systemctl enable sockproc
#systemctl enable redpitaya_scpi
EOF_CHROOT

################################################################################
# create users and groups
################################################################################

chroot $ROOT_DIR <<- EOF_CHROOT
# add system groups for running daemons
# for running bazar (Nginx), sockproc
useradd --system redpitaya_nginx
useradd --system scpi

# add HW access rights to Nginx user "redpitaya_nginx"
usermod -a -G xdevcfg,uio,xadc,led,gpio,spi,i2c,eeprom,dialout,dma redpitaya_nginx

# add HW access rights to users "scpi"
usermod -a -G uio,xadc,led,gpio,spi,i2c,eeprom,dialout,dma scpi

# TODO: Bazaar code should be moved from /dev/mem to /dev/uio/*
usermod -a -G kmem redpitaya
usermod -a -G kmem redpitaya_nginx
usermod -a -G kmem scpi
EOF_CHROOT
