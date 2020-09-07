#!/bin/bash
source ./sub_test/common_func.sh

USB_DEVICE="/dev/sda1"
USB_MOUNT_FOLDER="/mnt/usb"
USB_FILENAME='usb_device_testfile.txt'
USB_NEWFILENAME='usb_device_newname.txt'

echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#            Test of USB data                                          #\e[0m"
echo -e "\e[94m########################################################################\e[0m"
echo

STATUS=0

echo
USB_IN=$(fdisk -l | grep $USB_DEVICE)

if [ $? -ne 0 ]
then
    echo -n "    USB device not found, check if USB device is connected "
    print_fail
    STATUS=1

else

    # USB_DEVICE="/dev/sda1"
    # USB_MOUNT_FOLDER="/mnt/usb"
    # Create the mounting point folder and mount the device
    mkdir $USB_MOUNT_FOLDER > /dev/null 2>&1
    mount $USB_DEVICE $USB_MOUNT_FOLDER > /dev/null 2>&1

    if [ $? -ne 0 ]
    then
        echo -n "    Not possible to mount USB device "
        print_fail
        rm -rf $USB_MOUNT_FOLDER
        STATUS=1

    else
        # Move the file to /home and back with different name
        cp $USB_MOUNT_FOLDER/$USB_FILENAME /home
        mv /home/$USB_FILENAME $USB_MOUNT_FOLDER/$USB_NEWFILENAME

        # Compare the files and check they are the same
        cmp $USB_MOUNT_FOLDER/$USB_FILENAME $USB_MOUNT_FOLDER/$USB_NEWFILENAME

        if [ $? -eq 0 ]
        then
            echo -n "    Read & Write test on USB DRIVE "
            print_ok
        else
            echo -n "    Read & Write test on USB DRIVE "
            print_fail
            STATUS=1            
        fi
        # remove the new file, umount the device and delete the folder
        rm $USB_MOUNT_FOLDER/$USB_NEWFILENAME
        umount $USB_MOUNT_FOLDER
        rm -rf $USB_MOUNT_FOLDER
    fi
fi

echo
if [[ $STATUS == 0 ]]
then
    print_test_ok
    SetBitState 0x08
    RPLight3
else
    print_test_fail
fi

sleep 1

exit $STATUS