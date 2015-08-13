#!/bin/bash

#Red Pitaya SDK application run script
#
# @brief: This script copies the given code to a debian based Red pitaya
#	  and tries to compile the code. It then returns success or if there was
#         an error. Output is also saved in /tmp/log/sdk_log

GREET_MSG="\nEXECUTING RED PITAYA COMPILE SCRIPT..."
LINARO_HF_DL="https://releases.linaro.org/14.11/components/toolchain/binaries/arm-linux-gnueabihf/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf.tar.xz"

LINARO_HF_TAR = "gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf.tar.xz"
RP_INCLUDE=./include

#Installing dependency tools
echo -e $GREET_MSG
echo -e "INSTALLING NANO...\n"
sudo apt-get install nano
echo -e "INSTALLING CURL...\n"
sudo apt-get install curl
echo -e "INSTALLING PLINK...\n"
sudo apt-get install putty-tools

#Installing compilement tools
echo -e "INSTALLING COMPILE GCC LINARO HF...\n"
curl -L --remote-name $LINARO_HF_DL #Permission feedback required
sudo tar xvf $LINARO_HF_TAR
LINARO_HF_SUB=$LINARO_HF_DL | sed -e 's/.tar.xz//'
sudo cp -ar $LINARO_HF_SUB/* /opt/

echo -e "CREATING LINARO DIRECTORY..."
cd /opt; mkdir -p linaro; 
sudo cp -ar $LINARO_HF_SUB/* linaro
rm -rf ../$LINARO_HF_SUB
cd -;
sudo rm -rf $LINARO_HF_SUB
sudo rm -rf $LINARO_HF_DL 


