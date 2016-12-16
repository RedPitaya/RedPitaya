#!/bin/sh
################################################################################
# Author:
# Iztok Jeras <iztok.jeras@redpitaya.com>
# License:
# https://raw.githubusercontent.com/RedPitaya/RedPitaya/master/COPYING
################################################################################

################################################################################
# mounting image
################################################################################

IMAGE=$1
ECOSYSTEM=$2

BOOT_DIR=boot

DEVICE=`losetup -f`

mkdir $BOOT_DIR
losetup -P $DEVICE $IMAGE

BOOT_DEV=/dev/`lsblk -lno NAME -x NAME $DEVICE | sed '2!d'`
ROOT_DEV=/dev/`lsblk -lno NAME -x NAME $DEVICE | sed '3!d'`

mount -t vfat $BOOT_DEV $BOOT_DIR

################################################################################
# install ecosystem
################################################################################

# TODO: a format or a zeroing dd would be more appropriate
# remove old ecosystem files
rm -rf $BOOT_DIR/*

# Copy files to the boot file system
unzip $ECOSYSTEM -d $BOOT_DIR

################################################################################
# umount image
################################################################################

# Unmount file systems
umount $BOOT_DIR
rmdir $BOOT_DIR

losetup -d $DEVICE
