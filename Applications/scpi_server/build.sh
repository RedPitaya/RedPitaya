source /opt/Xilinx/Vivado/2015.4/settings64.sh
export TOOLCHAIN_PATH=/opt/linaro/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf
export PATH=$TOOLCHAIN_PATH/bin:$PATH
export CROSS_COMPILE=arm-linux-gnueabihf-
export BR2_DL_DIR=`pwd`/workspace/buildroot/dl
export SYSROOT=`pwd`/../../OS/buildroot/buildroot-2014.02/output/host/usr/arm-buildroot-linux-gnueabihf/sysroot
mkdir -p $BR2_DL_DIR
make
