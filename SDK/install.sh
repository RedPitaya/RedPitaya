#!/bin/bash

#Redpitaya INSTALL SDK install script

NAME=SDK
GREET_MSG="\nSTARTING REDPITAYA SDK INSTALLATION..."
LIB_MSG="\nINSTALLING REDPITAYA LIBRARIES...\n"
GCC_LINARO_MSG="\nINSTALLING GCC LINARO...\n"

RP_INCLUDE=./include
ECLIPSE_DL=eclipse-jee-luna-SR1a-linux-gtk-x86_64.tar.gz
GCC_LINARO_DL=gcc-linaro-arm-linux-gnueabi-2012.03-20120326_linux
GCC_LINARO_DIR=./gcc_linaro/bin


echo -e $GREET_MSG
echo "DOWNLOADING CURL...\n"
sudo apt-get install curl

echo -e "\nINSTALLING DEPENDENCIES...\n"
sudo apt-get install openjdk-7-jre
sudo chmod 777 /etc/apt/sources.list.d/ia32-libs-raring.list
sudo echo "deb http://old-releases.ubuntu.com/ubuntu/ raring main restricted universe multiverse" > /etc/apt/sources.list.d/ia32-libs-raring.list
sudo apt-get update
sudo apt-get install ia32-libs

echo -e "\nDOWNLOADING ECLIPSE...\n"
curl --remote-name http://mirrors.nic.cz/eclipse/technology/epp/downloads/\
release/luna/SR1a/$ECLIPSE_DL

tar xvf $ECLIPSE_DL eclipse
rm $ECLIPSE_DL

echo -e $LIB_MSG
sudo cp $RP_INCLUDE/* /usr/lib
sudo cp $RP_INCLUDE/* /usr/include

echo -e $GCC_LINARO_MSG
curl -L --remote-name https://launchpad.net/linaro-toolchain-binaries/trunk/2012.03/+download/$GCC_LINARO_DL.tar.bz2

tar xvf $GCC_LINARO_DL.tar.bz2
sudo mv $GCC_LINARO_DL gcc_linaro
rm -rf $GCC_LINARO_DL.tar.bz2

sudo chmod 777 /etc/bash.bashrc
echo export PATH=$PATH:$PWD/$GCC_LINARO_DIR


