#!/bin/sh
################################################################################
# Authors:
# - Pavel Demin <pavel.demin@uclouvain.be>
# - Iztok Jeras <iztok.jeras@redpitaya.com>
# License:
# https://raw.githubusercontent.com/RedPitaya/RedPitaya/master/COPYING
################################################################################

################################################################################
# mounting image
################################################################################

IMAGE=$1

BOOT_DIR=boot

DEVICE=`losetup -f`

mkdir $BOOT_DIR
losetup --offset 4194304 $DEVICE $IMAGE

mount -t vfat $DEVICE $BOOT_DIR

################################################################################
# install ecosystem
################################################################################

# reove old ecosystem files
rm -rf $BOOT_DIR/*

# Copy files to the boot file system
unzip ecosystem*.zip -d $BOOT_DIR

################################################################################
# umount image
################################################################################

# Unmount file systems
umount $BOOT_DIR
rmdir $BOOT_DIR

losetup -d $DEVICE
