#!/bin/sh
################################################################################
#
################################################################################

################################################################################
# mounting image
################################################################################

IMAGE=$1

DEVICE=`losetup -f`

losetup $DEVICE $IMAGE

BOOT_DIR=boot
ROOT_DIR=root

BOOT_DEV=/dev/`lsblk -lno NAME $DEVICE | sed '2!d'`
ROOT_DEV=/dev/`lsblk -lno NAME $DEVICE | sed '3!d'`

# Mount file systems
mkdir -p $BOOT_DIR $ROOT_DIR
mount $BOOT_DEV $BOOT_DIR
mount $ROOT_DEV $ROOT_DIR

################################################################################
# install OS
################################################################################

OVERLAY=OS/debian/overlay

#source OS/debian/debian.sh 

# enable chroot access with native execution
cp /etc/resolv.conf         $ROOT_DIR/etc/
cp /usr/bin/qemu-arm-static $ROOT_DIR/usr/bin/

#source OS/debian/redpitaya.sh
. OS/debian/wyliodrin.sh

# disable chroot access with native execution
rm $ROOT_DIR/etc/resolv.conf
rm $ROOT_DIR/usr/bin/qemu-arm-static

################################################################################
# umount image
################################################################################

# Unmount file systems
umount $BOOT_DIR $ROOT_DIR
rmdir $BOOT_DIR $ROOT_DIR

losetup -d $DEVICE
