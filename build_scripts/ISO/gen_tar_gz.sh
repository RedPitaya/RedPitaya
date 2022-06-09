#!/bin/bash

if [ -z "$1" ]
then
    echo "Missing ecosystem file name as parameter"
    exit 1
fi

IMG=$1
FILE_NAME=$(mktemp)
cp $IMG $FILE_NAME

mkdir -p boot root
LOOP_DEV=`sudo losetup -fP --show "$FILE_NAME"`
echo "$LOOP_DEV"
echo "mount img"
sudo mount -o rw "$LOOP_DEV"p2 root

LC_TIME="en_US.UTF-8"
DATE=`date +"%d-%b-%Y" | tr '[:upper:]' '[:lower:]'`


rm root/etc/resolv.conf
sync
tar -cpzf redpitaya_ubuntu_${DATE}.tar.gz --one-file-system -C root .

echo "umount img"
sudo umount  ./root
sudo rm -rf  ./root
sudo losetup -d "$LOOP_DEV"
rm $FILE_NAME

echo "ALL DONE"
