################################################################################
# setup Xilinx Vivado FPGA tools
################################################################################

. /opt/Xilinx/Vivado/2015.4/settings64.sh

################################################################################
# setup Linaro toolchain
################################################################################

#export TOOLCHAIN_PATH=/opt/linaro/gcc-linaro-4.9-2015.02-3-x86_64_arm-linux-gnueabihf
export TOOLCHAIN_PATH=/opt/linaro/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf/
export PATH=$PATH:$TOOLCHAIN_PATH/bin
export CROSS_COMPILE=arm-linux-gnueabihf-

################################################################################
# setup Buildroot download cache directory, to avoid downloads
# this path is also used by some other downloads
################################################################################

#export BR2_DL_DIR=$HOME/Workplace/buildroot/dl
export BR2_DL_DIR=dl

################################################################################
# common make procedure, should not be run by this script
################################################################################

#GIT_COMMIT_SHORT=`git rev-parse --short HEAD`
#make REVISION=$GIT_COMMIT_SHORT
