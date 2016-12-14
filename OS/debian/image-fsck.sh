#!/bin/sh
################################################################################
# Authors:
# - Iztok Jeras <iztok.jeras@redpitaya.com>
# License:
# https://raw.githubusercontent.com/RedPitaya/RedPitaya/master/COPYING
################################################################################

################################################################################
# file system check
################################################################################

IMAGE=$1

DEVICE=`losetup -f`

losetup -P $DEVICE $IMAGE

BOOT_DEV=/dev/`lsblk -lno NAME -x NAME $DEVICE | sed '2!d'`
ROOT_DEV=/dev/`lsblk -lno NAME -x NAME $DEVICE | sed '3!d'`

fsck.vfat -n  $BOOT_DEV
fsck.ext4 -nf $ROOT_DEV

losetup -d $DEVICE
