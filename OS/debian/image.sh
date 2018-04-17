#!/bin/sh
################################################################################
# Authors:
# - Pavel Demin <pavel.demin@uclouvain.be>
# - Iztok Jeras <iztok.jeras@redpitaya.com>
# License:
# https://raw.githubusercontent.com/RedPitaya/RedPitaya/master/COPYING
################################################################################

# Optional system variables:
# TIMEZONE - it is written into /etc/timezone
# 


################################################################################
# prepating image
################################################################################

# current time and date are used to create the image name
DATE=`date +"%H-%M-%S_%d-%b-%Y"`

# default image size if 3GB, which is appropriate for all 4BG SD cards
SIZE=3500

#IMAGE=$1
IMAGE=redpitaya_OS_${DATE}.img

dd if=/dev/zero of=$IMAGE bs=1M count=$SIZE

DEVICE=`losetup -f`

losetup $DEVICE $IMAGE

BOOT_DIR=boot
ROOT_DIR=root

# Create partitions
parted -s $DEVICE mklabel msdos
parted -s $DEVICE mkpart primary fat16   4MB 128MB
parted -s $DEVICE mkpart primary ext4  128MB 100%

BOOT_DEV=/dev/`lsblk -lno NAME -x NAME $DEVICE | sed '2!d'`
ROOT_DEV=/dev/`lsblk -lno NAME -x NAME $DEVICE | sed '3!d'`

# Create file systems
mkfs.vfat -v    $BOOT_DEV
mkfs.ext4 -F -j $ROOT_DEV

################################################################################
# mount image
################################################################################

# Mount file systems
mkdir -p $BOOT_DIR $ROOT_DIR
mount $BOOT_DEV $BOOT_DIR
mount $ROOT_DEV $ROOT_DIR

################################################################################
# install OS
################################################################################

. OS/debian/ubuntu.sh 2>&1 | tee $ROOT_DIR/buildlog.txt
#. OS/debian/debian.sh 2>&1 | tee $ROOT_DIR/buildlog.txt

################################################################################
# umount image
################################################################################

# Unmount file systems
umount $BOOT_DIR $ROOT_DIR
rmdir $BOOT_DIR $ROOT_DIR

losetup -d $DEVICE
