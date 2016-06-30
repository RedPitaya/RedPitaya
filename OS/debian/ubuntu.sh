################################################################################
# Authors:
# - Pavel Demin <pavel.demin@uclouvain.be>
# - Iztok Jeras <iztok.jeras@redpitaya.com>
# License:
# https://raw.githubusercontent.com/RedPitaya/RedPitaya/master/COPYING
################################################################################

# Install Ubuntu base system to the root file system
UBUNTU_BASE_TAR=ubuntu-base-16.04-core-armhf.tar.gz
UBUNTU_BASE_URL=http://cdimage.ubuntu.com/ubuntu-base/releases/16.04/release/$UBUNTU_BASE_TAR
test -f $UBUNTU_BASE_TAR || curl -L $UBUNTU_BASE_URL -o $UBUNTU_BASE_TAR
tar -zxf $UBUNTU_BASE_TAR --directory=$ROOT_DIR

OVERLAY=OS/debian/overlay

# enable chroot access with native execution
cp /etc/resolv.conf         $ROOT_DIR/etc/
cp /usr/bin/qemu-arm-static $ROOT_DIR/usr/bin/

# copy U-Boot environment tools
install -v -m 664 -o root -D patches/fw_env.config                      $ROOT_DIR/etc/fw_env.config

install -v -m 664 -o root -D $OVERLAY/etc/apt/apt.conf.d/99norecommends $ROOT_DIR/etc/apt/apt.conf.d/99norecommends
install -v -m 664 -o root -D $OVERLAY/etc/apt/sources.list              $ROOT_DIR/etc/apt/sources.list
install -v -m 664 -o root -D $OVERLAY/etc/fstab                         $ROOT_DIR/etc/fstab
install -v -m 664 -o root -D $OVERLAY/etc/hostname                      $ROOT_DIR/etc/hostname
install -v -m 664 -o root -D $OVERLAY/etc/timezone                      $ROOT_DIR/etc/timezone
install -v -m 664 -o root -D $OVERLAY/etc/securetty                     $ROOT_DIR/etc/securetty

chroot $ROOT_DIR <<- EOF_CHROOT
apt-get update
apt-get -y upgrade
apt-get -y install dbus udev

# setup hostname and timezone
# TODO seems sytemd is not running without /proc/cmdline or something
#hostnamectl set-hostname redpitaya
#timedatectl set-timezone Europe/Ljubljana

# setup locale
apt-get -y install locales
sed -i "/^# en_US.UTF-8 UTF-8$/s/^# //" /etc/locale.gen
locale-gen
update-locale LANG=en_US.UTF-8

# setup locale/keyboard
#apt-get -y install locales console-data keyboard-configuration
#dpkg-reconfigure keyboard-configuration
#localectl   set-locale   LANG="en_US.utf8"

dpkg-reconfigure --frontend=noninteractive tzdata

# add package containing add-apt-repository
#apt-get -y install software-properties-common
#add-apt-repository -yu ppa:redpitaya/zynq

# development tools
apt-get -y install build-essential less vim nano sudo u-boot-tools usbutils psmisc lsof
apt-get -y install parted dosfstools
EOF_CHROOT

. OS/debian/network.sh
. OS/debian/redpitaya.sh
#. OS/debian/wyliodrin.sh

chroot $ROOT_DIR <<- EOF_CHROOT
echo root:root | chpasswd
apt-get clean
history -c
EOF_CHROOT

################################################################################
# cleanup
################################################################################

# file system cleanup for better compression
cat /dev/zero > $ROOT_DIR/zero.file
sync -f $ROOT_DIR/zero.file
rm -f $ROOT_DIR/zero.file

# remove ARM emulation
rm $ROOT_DIR/usr/bin/qemu-arm-static

# disable chroot access with native execution
rm $ROOT_DIR/etc/resolv.conf
# create a tarball (without resolv.conf link, since it causes schroot issues)
tar -cpzf redpitaya_ubuntu_${DATE}.tar.gz --one-file-system -C $ROOT_DIR .
# create resolv.conf link
ln -sf /run/systemd/resolve/resolv.conf $ROOT_DIR/etc/resolv.conf
