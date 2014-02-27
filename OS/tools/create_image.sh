#!/bin/sh

[ $# -lt 1 ] && {
    echo Usage: $0 ramdisk.img
    exit 255
}

cp $1 tmp.image
gzip -9 tmp.image
mkimage -A arm -T ramdisk -C none -n ramdisk -d tmp.image.gz uramdisk.image.gz

rm tmp.image.gz

echo
echo "Created ramdisk image: uramdisk.image.gz"
echo
