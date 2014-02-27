#!/bin/sh

[ $# -lt 1 ] && {
    echo Usage: $0 ramdisk.img.gz
    exit 255
}

dd bs=1 skip=64 if=$1 of=ramdisk.img.gz
zcat ramdisk.img.gz > ramdisk.img

rm -f ramdisk.img.gz

echo
echo "Extracted ramdisk image: ramdisk.img"
echo "Mount it with:"
echo
echo "    sudo mount -t ext2 -o loop ramdisk.img /mnt"
echo
