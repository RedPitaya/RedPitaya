################################################################################
# setup Xilinx Vivado FPGA tools
################################################################################

. /opt/Xilinx/Vivado/2015.3/settings64.sh

################################################################################
# setup Linaro toolchain
################################################################################

#export TOOLCHAIN_PATH=/opt/linaro/gcc-linaro-4.9-2015.02-3-x86_64_arm-linux-gnueabihf
export TOOLCHAIN_PATH=/opt/linaro/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf/
export PATH=$TOOLCHAIN_PATH/bin:$PATH
export CROSS_COMPILE=arm-linux-gnueabihf-

################################################################################
# setup Buildroot download cache directory, to avoid downloads
# this path is also used by some other downloads
################################################################################

export BR2_DL_DIR=dl

################################################################################
# which boot image / file system should be entered? Select one of them
# When both are selected, each one is prepared but Debian wins for the FSBL
################################################################################

#export FSBL_BR=1
export FSBL_DEBIAN=1

################################################################################
# common make procedure, should not be run by this script
################################################################################

#GIT_COMMIT_SHORT=`git rev-parse --short HEAD`
#make REVISION=$GIT_COMMIT_SHORT
