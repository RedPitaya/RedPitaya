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

# Create file systems
mkfs.vfat -v    $BOOT_DEV

# mount
mkdir -p $BOOT_DIR
mount -t vfat $BOOT_DEV $BOOT_DIR

################################################################################
# install ecosystem
################################################################################

# Copy files to the boot file system
unzip $ECOSYSTEM -d $BOOT_DIR

################################################################################
# umount image
################################################################################

# Unmount file systems
umount $BOOT_DIR
rmdir $BOOT_DIR

losetup -d $DEVICE
