################################################################################
# Authors:
# - Pavel Demin <pavel.demin@uclouvain.be>
# - Iztok Jeras <iztok.jeras@redpitaya.com>
# License:
# https://raw.githubusercontent.com/RedPitaya/RedPitaya/master/COPYING
################################################################################

# Copy files to the boot file system
unzip ecosystem*.zip -d $BOOT_DIR

# Systemd services
install -v -m 664 -o root -d                                                         $ROOT_DIR/var/log/redpitaya_nginx
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/redpitaya_discovery.service $ROOT_DIR/etc/systemd/system/redpitaya_discovery.service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/redpitaya_nginx.service     $ROOT_DIR/etc/systemd/system/redpitaya_nginx.service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/sockproc.service            $ROOT_DIR/etc/systemd/system/sockproc.service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/redpitaya_scpi.service      $ROOT_DIR/etc/systemd/system/redpitaya_scpi.service
install -v -m 664 -o root -D $OVERLAY/etc/sysconfig/redpitaya                        $ROOT_DIR/etc/sysconfig/redpitaya
# TODO: this Wyliodrin service is only here since wyliodrin.sh can not be run in a virtualized environment
# Wyliodrin service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/redpitaya_wyliodrin.service $ROOT_DIR/etc/systemd/system/redpitaya_wyliodrin.service

chroot $ROOT_DIR <<- EOF_CHROOT
systemctl enable redpitaya_discovery
systemctl enable redpitaya_nginx
systemctl enable sockproc
#systemctl enable redpitaya_scpi

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

# libraries used by lcrmeter
sudo apt-get install -y libi2c-dev i2c-tools

# tools used to compile applications
apt-get -y install zip

# debug tools
apt-get -y install gdb cgdb libcunit1-ncurses-dev

EOF_CHROOT

# profile for PATH variables, ...
install -v -m 664 -o root -D $OVERLAY/etc/profile.d/profile.sh   $ROOT_DIR/etc/profile.d/profile.sh
install -v -m 664 -o root -D $OVERLAY/etc/profile.d/alias.sh     $ROOT_DIR/etc/profile.d/alias.sh
install -v -m 664 -o root -D $OVERLAY/etc/profile.d/redpitaya.sh $ROOT_DIR/etc/profile.d/redpitaya.sh

# remove existing MOTD and replace it with a link to Red Pitaya version.txt
# TODO this approach worked on Debian but does not work well on Ubuntu 16.04
#rm $ROOT_DIR/etc/motd
#ln -s /opt/redpitaya/version.txt $ROOT_DIR/etc/motd 
