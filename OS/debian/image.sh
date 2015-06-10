#!/bin/bash
################################################################################
#
################################################################################

################################################################################
# prepating and mounting image
################################################################################

#image=$1
image=debian_armel.img

#size=$2
size=1024

dd if=/dev/zero of=$image bs=1M count=$size

device=`losetup -f`

losetup $device $image

boot_dir=BOOT
root_dir=ROOT

# Create partitions
parted -s $device mklabel msdos
parted -s $device mkpart primary fat16   4MB 128MB
parted -s $device mkpart primary ext4  128MB 100%

boot_dev=/dev/`lsblk -lno NAME $device | sed '2!d'`
root_dev=/dev/`lsblk -lno NAME $device | sed '3!d'`

# Create file systems
mkfs.vfat -v    $boot_dev
mkfs.ext4 -F -j $root_dev

# Mount file systems
mkdir -p $boot_dir $root_dir
mount $boot_dev $boot_dir
mount $root_dev $root_dir

################################################################################
# install OS
################################################################################

OVERLAY=OS/debian/overlay

source OS/debian/debian.sh
source OS/debian/redpitaya.sh
#source OS/debian/wyliodrin.sh

################################################################################
# umount image
################################################################################

# Unmount file systems
umount $boot_dir $root_dir
rmdir $boot_dir $root_dir

losetup -d             $device
