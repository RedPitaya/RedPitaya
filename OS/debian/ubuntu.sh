################################################################################
# Authors:
# - Pavel Demin <pavel.demin@uclouvain.be>
# - Iztok Jeras <iztok.jeras@redpitaya.com>
# License:
# https://raw.githubusercontent.com/RedPitaya/RedPitaya/master/COPYING
################################################################################

# Install Ubuntu base system to the root file system
UBUNTU_BASE_VER=16.04.2
UBUNTU_BASE_TAR=ubuntu-base-${UBUNTU_BASE_VER}-base-armhf.tar.gz
UBUNTU_BASE_URL=http://cdimage.ubuntu.com/ubuntu-base/releases/${UBUNTU_BASE_VER}/release/${UBUNTU_BASE_TAR}
test -f $UBUNTU_BASE_TAR || curl -L $UBUNTU_BASE_URL -o $UBUNTU_BASE_TAR
tar -zxf $UBUNTU_BASE_TAR --directory=$ROOT_DIR

OVERLAY=OS/debian/overlay

# enable chroot access with native execution
cp /etc/resolv.conf         $ROOT_DIR/etc/
cp /usr/bin/qemu-arm-static $ROOT_DIR/usr/bin/

export LC_ALL=en_US.UTF-8

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
# locale and keyboard
# setting LC_ALL overides values for all LC_* variables, this avids complaints
# about missing locales if some of this variables are inherited over SSH
################################################################################

chroot $ROOT_DIR <<- EOF_CHROOT
# this is needed by systemd services 'keyboard-setup.service' and 'console-setup.service'
DEBIAN_FRONTEND=noninteractive \
apt-get -y install console-setup

# setup locale
apt-get -y install locales
locale-gen en_US.UTF-8
update-locale LANG=en_US.UTF-8 LANGUAGE=en_US LC_ALL=en_US.UTF-8

# TODO seems sytemd is not running without /proc/cmdline or something
#localectl set-locale LANG=en_US.UTF-8 LANGUAGE=en_US LC_ALL=en_US.UTF-8
#localectl set-keymap us

# Debug log
locale -a
locale
cat /etc/default/locale
cat /etc/default/keyboard
EOF_CHROOT

################################################################################
# hostname
# NOTE: redpitaya.py enables a systemd service
# which changes the hostname on boot, to an unique value
################################################################################

#chroot $ROOT_DIR <<- EOF_CHROOT
# TODO seems sytemd is not running without /proc/cmdline or something
#hostnamectl set-hostname redpitaya
#EOF_CHROOT

install -v -m 664 -o root -D $OVERLAY/etc/hostname  $ROOT_DIR/etc/hostname

################################################################################
# timezone and fake HW time
################################################################################

chroot $ROOT_DIR <<- EOF_CHROOT
# install fake hardware clock
apt-get -y install fake-hwclock

dpkg-reconfigure --frontend=noninteractive tzdata

# TODO seems sytemd is not running without /proc/cmdline or something
#timedatectl set-timezone Europe/Ljubljana
EOF_CHROOT

# set default timezone
if [ "$TIMEZONE" = "" ]; then
  TIMEZONE="Etc/UTC"
fi
# set timezone and fake RTC time
echo timezone = $TIMEZONE
echo $TIMEZONE > $ROOT_DIR/etc/timezone

# the fake HW clock will be UTC, so an adjust file is not needed
#echo $MYADJTIME > $ROOT_DIR/etc/adjtime
# fake HW time is set to the image build time
DATETIME=`date -u +"%F %T"`
echo date/time = $DATETIME
echo $DATETIME > $ROOT_DIR/etc/fake-hwclock.data

################################################################################
# File System table
################################################################################

install -v -m 664 -o root -D $OVERLAY/etc/fstab  $ROOT_DIR/etc/fstab

################################################################################
# run other scripts
################################################################################

. OS/debian/tools.sh
. OS/debian/network.sh
. OS/debian/zynq.sh
. OS/debian/redpitaya.sh
. OS/debian/jupyter.sh
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
