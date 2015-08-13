#!/bin/bash

#Red Pitaya SDK application run script
#
# @brief: This script copies the given code to a debian based Red pitaya
#	  and tries to compile the code. It then returns success or if there was
#         an error. Output is also saved in /tmp/log/sdk_log

GREET_MSG="\nEXECUTING RED PITAYA COMPILE SCRIPT..."
LINARO_HF_DL="https://releases.linaro.org/14.11/components/toolchain/binaries/arm-linux-gnueabihf/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf.tar.xz"

LINARO_HF_TAR=gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf.tar.xz
EXEC_PATH="/opt/linaro/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf/bin"

#Installing dependency tools
echo -e $GREET_MSG
echo -e "\nINSTALLING NANO..."
sudo apt-get install nano
echo -e "\nINSTALLING CURL..."
sudo apt-get install curl
echo -e "\nINSTALLING PLINK..."
sudo apt-get install putty-tools
echo -e

#Installing compilement tools
echo -e "\nINSTALLING COMPILE GCC LINARO HF..."
curl -L --remote-name $LINARO_HF_DL #Permission feedback required
sudo tar xvf $LINARO_HF_TAR
LINARO_HF_SUB=$LINARO_HF_TAR | sed -e 's/.tar.xz//'

echo -e "\nCREATING LINARO ROOT DIRECTORIES..."
cd /opt; mkdir -p linaro; 
cd -
echo -e "\nCOPYING DATA..."
pwd
echo $LINARO_HF_SUB
sudo cp -r $LINARO_HF_SUB/* /opt/linaro
sudo rm -rf $LINARO_HF_SUB
sudo rm -rf $LINARO_HF_DL 

#Seting seassion environment
echo -e "\nSETTING ENVIRONMENTAL VARIABLES..."
export PATH:$EXEC_PATH
export CROSS_COMPILE=arm-linux-gnueabihf-
