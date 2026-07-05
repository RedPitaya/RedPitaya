#!/bin/bash

RP_UBUNTU=red_pitaya_OS_3.00.36.tar.gz
SCHROOT_CONF_PATH=/etc/schroot/chroot.d/red-pitaya-ubuntu.conf

function print_ok(){
    echo -e "\033[92m[OK]\e[0m"
}

function print_fail(){
    echo -e "\033[91m[FAIL]\e[0m"
}

echo Start build process...
# Check Xilinx vivado path


echo
echo "Setup development packages"

# generic dependencies
sudo apt-get install make curl xz-utils -y
# U-Boot build dependencies
sudo apt-get install libssl-dev device-tree-compiler u-boot-tools -y
# secure chroot
sudo apt-get install schroot -y
# QEMU
sudo apt-get install qemu-user qemu-user-static -y

sudo apt-get install device-tree-compiler -y

sleep 1
echo
echo -n "Complete development packages "
print_ok

cd ..

export DL=${PWD}/tmp/DL

mkdir -p $DL
echo -n "Created directory for download. "
if [[ -d "$DL" ]]
then
print_ok
else
print_fail
exit 1
fi

if [ -z "$1" ]
then
    echo -n "Download redpitaya ubuntu OS. "
    cd $DL
    wget -N http://downloads.redpitaya.com/downloads/LinuxOS/$RP_UBUNTU
else
    echo "Set ubuntu OS from parameter $1"
    RP_UBUNTU=$1
    cd build_scripts
    cp -f $RP_UBUNTU $DL/$RP_UBUNTU
    cd $DL
fi


echo -n "Check redpitaya ubuntu OS. "
if [[ -f "$RP_UBUNTU" ]]
        chown root:root $RP_UBUNTU
        chmod 664 $RP_UBUNTU
then
print_ok
else
print_fail
exit 1
fi

cd ../..

if [[ -f "$SCHROOT_CONF_PATH" ]]
then
echo "File $SCHROOT_CONF_PATH is exists"
sudo rm -f $SCHROOT_CONF_PATH
echo "File $SCHROOT_CONF_PATH is deleted"
fi

sleep 1
echo  "Write new configuration"
echo
echo  "[red-pitaya-ubuntu]"      | sudo tee -a $SCHROOT_CONF_PATH
echo  "description=Red pitaya"   | sudo tee -a $SCHROOT_CONF_PATH
echo  "type=file"                | sudo tee -a $SCHROOT_CONF_PATH
echo  "file=$DL/$RP_UBUNTU"      | sudo tee -a $SCHROOT_CONF_PATH
echo  "users=root"               | sudo tee -a $SCHROOT_CONF_PATH
echo  "root-users=root"          | sudo tee -a $SCHROOT_CONF_PATH
echo  "root-groups=root"         | sudo tee -a $SCHROOT_CONF_PATH
echo  "personality=linux"        | sudo tee -a $SCHROOT_CONF_PATH
echo  "preserve-environment=true"| sudo tee -a $SCHROOT_CONF_PATH
if [[ $? = 0 ]]
then
echo
echo -n "Complete write new configuration "
print_ok
echo
else
echo -n "Complete write new configuration "
print_fail
exit 1
fi

set -e
pwd
chmod +x ./settings.sh
. settings.sh
echo -n "Call settings.sh "
print_ok

export CROSS_COMPILE=arm-linux-gnueabihf-
export ARCH=arm

GIT_COMMIT_SHORT=`git rev-parse --short HEAD`

make -f Makefile.x86 fpga MODEL=Z10
make -f Makefile.x86 fpga MODEL=Z20
make -f Makefile.x86 fpga MODEL=Z20_125
make -f Makefile.x86 fpga MODEL=Z20_125_4CH
make -f Makefile.x86 fpga MODEL=Z20_250_12
make -f Makefile.x86 fpga MODEL=Z20_250_12a
make -f Makefile.x86 fpga MODEL=Z10_V2
make -f Makefile.x86 fpga MODEL=Z10_PRO_V2
make -f Makefile.x86 fpga MODEL=Z20_125_V2
make -f Makefile.x86 fpga MODEL=Z20_LL

make -f Makefile.x86

schroot -c red-pitaya-ubuntu <<- EOL_CHROOT
make -f Makefile CROSS_COMPILE="" REVISION=$GIT_COMMIT_SHORT ENABLE_PRODUCTION_TEST=0 ENABLE_LICENSING=0 BUILD_NUMBER=1
EOL_CHROOT

# required min-gw
#make -f Makefile.x86 streaming

# FOR BUILD QT CLIENTS NEED SETUP QT 5.15.2
#make -f Makefile.x86 streaming_client_qt
#export QT_DIR=/srv/Qt5.15.2-win
#make -f Makefile.x86 streaming_client_qt_win
#unset QT_DIR

make -f Makefile.x86 zip
