################################################################################
# Authors:
# - Pavel Demin <pavel.demin@uclouvain.be>
# - Iztok Jeras <iztok.jeras@redpitaya.com>
# License:
# https://raw.githubusercontent.com/RedPitaya/RedPitaya/master/COPYING
################################################################################

# Install Ubuntu base system to the root file system
UBUNTU_BASE_VER=16.04.1
UBUNTU_BASE_TAR=ubuntu-base-${UBUNTU_BASE_VER}-base-armhf.tar.gz
UBUNTU_BASE_URL=http://cdimage.ubuntu.com/ubuntu-base/releases/${UBUNTU_BASE_VER}/release/${UBUNTU_BASE_TAR}
test -f $UBUNTU_BASE_TAR || curl -L $UBUNTU_BASE_URL -o $UBUNTU_BASE_TAR
tar -zxf $UBUNTU_BASE_TAR --directory=$ROOT_DIR

OVERLAY=OS/debian/overlay

# enable chroot access with native execution
cp /etc/resolv.conf         $ROOT_DIR/etc/
cp /usr/bin/qemu-arm-static $ROOT_DIR/usr/bin/

################################################################################
# APT settings
################################################################################

install -v -m 664 -o root -D $OVERLAY/etc/apt/apt.conf.d/99norecommends $ROOT_DIR/etc/apt/apt.conf.d/99norecommends
install -v -m 664 -o root -D $OVERLAY/etc/apt/sources.list              $ROOT_DIR/etc/apt/sources.list

chroot $ROOT_DIR <<- EOF_CHROOT
apt-get update
apt-get -y upgrade

# add package containing add-apt-repository
apt-get -y install software-properties-common
# add PPA: https://launchpad.net/~redpitaya/+archive/ubuntu/zynq
add-apt-repository -yu ppa:redpitaya/zynq
EOF_CHROOT

################################################################################
# miscelaneous tools
################################################################################

chroot $ROOT_DIR <<- EOF_CHROOT
apt-get -y install dbus udev

# development tools
apt-get -y install build-essential less vim nano sudo usbutils psmisc lsof
apt-get -y install parted dosfstools

# install file system tools
apt-get -y install mtd-utils
EOF_CHROOT

################################################################################
# File System table
################################################################################

install -v -m 664 -o root -D $OVERLAY/etc/fstab  $ROOT_DIR/etc/fstab

################################################################################
# U-Boot environment EEPROM memory map
################################################################################

chroot $ROOT_DIR <<- EOF_CHROOT
# development tools
apt-get -y install u-boot-tools
EOF_CHROOT

# copy U-Boot environment tools
install -v -m 664 -o root -D patches/fw_env.config  $ROOT_DIR/etc/fw_env.config

################################################################################
# hostname
################################################################################

install -v -m 664 -o root -D $OVERLAY/etc/hostname  $ROOT_DIR/etc/hostname

chroot $ROOT_DIR <<- EOF_CHROOT
# TODO seems sytemd is not running without /proc/cmdline or something
#hostnamectl set-hostname redpitaya
EOF_CHROOT

################################################################################
# time and locale
################################################################################

# set timezone and fake RTC time
if [ "$TIMEZONE" = "" ]; then
  TIMEZONE="Europe/Ljubljana"
fi
echo timezone = $TIMEZONE
echo $TIMEZONE > $ROOT_DIR/etc/timezone

# the fake HW clock will be UTC, so an adjust file is not needed
#echo $MYADJTIME > $ROOT_DIR/etc/adjtime
# fake HW time is set to the image build time
DATETIME=`date -u +"%F %T"`
echo date/time = $DATETIME
echo $DATETIME > $ROOT_DIR/etc/fake-hwclock.data

chroot $ROOT_DIR <<- EOF_CHROOT
# install fake hardware clock
apt-get -y install fake-hwclock

dpkg-reconfigure --frontend=noninteractive tzdata

# TODO seems sytemd is not running without /proc/cmdline or something
#timedatectl set-timezone Europe/Ljubljana
EOF_CHROOT

################################################################################
# locale and keyboard
################################################################################

chroot $ROOT_DIR <<- EOF_CHROOT
# setup locale
apt-get -y install locales
sed -i "/^# en_US.UTF-8 UTF-8$/s/^# //" /etc/locale.gen
locale-gen
update-locale LANG=en_US.UTF-8

# setup locale/keyboard
#apt-get -y install locales console-data keyboard-configuration
#dpkg-reconfigure keyboard-configuration
#localectl   set-locale   LANG="en_US.utf8"
EOF_CHROOT

################################################################################
# run other scripts
################################################################################

. OS/debian/network.sh
. OS/debian/redpitaya.sh
#. OS/debian/wyliodrin.sh
#. OS/debian/tft.sh

################################################################################
# handle users
################################################################################

# http://0pointer.de/blog/projects/serial-console.html

install -v -m 664 -o root -D $OVERLAY/etc/securetty $ROOT_DIR/etc/securetty
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/serial-getty@ttyPS0.service.d/override.conf \
                            $ROOT_DIR/etc/systemd/system/serial-getty@ttyPS0.service.d/override.conf

chroot $ROOT_DIR <<- EOF_CHROOT
echo root:root | chpasswd
EOF_CHROOT

################################################################################
# cleanup
################################################################################

chroot $ROOT_DIR <<- EOF_CHROOT
apt-get clean
history -c
EOF_CHROOT

# kill -k file users and list them -m before Unmount file systems
fuser -km $BOOT_DIR
fuser -km $ROOT_DIR

# file system cleanup for better compression
cat /dev/zero > $ROOT_DIR/zero.file
sync -f $ROOT_DIR/zero.file
rm -f $ROOT_DIR/zero.file

# remove ARM emulation
rm $ROOT_DIR/usr/bin/qemu-arm-static

################################################################################
# archiving image
################################################################################

# create a tarball (without resolv.conf link, since it causes schroot issues)
rm $ROOT_DIR/etc/resolv.conf
tar -cpzf redpitaya_ubuntu_${DATE}.tar.gz --one-file-system -C $ROOT_DIR .
# recreate resolv.conf link
ln -sf /run/systemd/resolve/resolv.conf $ROOT_DIR/etc/resolv.conf

# one final sync to be sure
sync
