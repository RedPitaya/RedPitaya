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

BOOT_DIR=boot
ROOT_DIR=root

DEVICE=`losetup -f`

mkdir $BOOT_DIR
losetup -P $DEVICE $IMAGE

BOOT_DEV=/dev/`lsblk -lno NAME -x NAME $DEVICE | sed '2!d'`
ROOT_DEV=/dev/`lsblk -lno NAME -x NAME $DEVICE | sed '3!d'`

# Create file systems
mkfs.vfat -v    $BOOT_DEV

# mount
mkdir -p $BOOT_DIR $ROOT_DIR
mount -t vfat $BOOT_DEV $BOOT_DIR
mount -t ext4 $ROOT_DEV $ROOT_DIR

################################################################################
# clean
################################################################################

# removing OSX files
rm -rf $BOOT_DIR/.Spotlight-V100
rm -rf $BOOT_DIR/.Trashes
rm -rf $BOOT_DIR/System\ Volume\ Information

# VFAT file system check and repair
#fsck.vfat -al $BOOT_DEV

# remove SSH keys, so they can be created at boot by ssh-reconfigure.service
rm -v $ROOT_DIR/etc/ssh/ssh_host_*

# file system cleanup for better compression
cat /dev/zero > $ROOT_DIR/zero.file
sync -f $ROOT_DIR/zero.file
rm -f $ROOT_DIR/zero.file

################################################################################
# umount image
################################################################################

# Unmount file systems
umount $BOOT_DIR $ROOT_DIR
rmdir $BOOT_DIR $ROOT_DIR

losetup -d $DEVICE
