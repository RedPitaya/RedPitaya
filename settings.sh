################################################################################
# setup Xilinx Vivado FPGA tools
################################################################################

. /opt/Xilinx/Vivado/2016.1/settings64.sh

################################################################################
# setup cross compiler toolchain
################################################################################

#export TOOLCHAIN_PATH=/opt/linaro/gcc-linaro-4.9-2015.02-3-x86_64_arm-linux-gnueabihf
#export TOOLCHAIN_PATH=/opt/linaro/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf/
#export PATH=$TOOLCHAIN_PATH/bin:$PATH
export CROSS_COMPILE=arm-linux-gnueabihf-

################################################################################
# setup download cache directory, to avoid downloads
################################################################################

#export DL=dl

################################################################################
# common make procedure, should not be run by this script
################################################################################

#GIT_COMMIT_SHORT=`git rev-parse --short HEAD`
#make REVISION=$GIT_COMMIT_SHORT
