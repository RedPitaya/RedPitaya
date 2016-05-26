source /opt/Xilinx/Vivado/2015.2/settings64.sh
export TOOLCHAIN_PATH=/opt/linaro/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf
#export TOOLCHAIN_PATH=/opt/linaro/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf
#export TOOLCHAIN_PATH=/home/user/gcc-linaro-5.1-2015.08-x86_64_arm-linux-gnueabihf
export PATH=$TOOLCHAIN_PATH/bin:$PATH
export CROSS_COMPILE=arm-linux-gnueabihf-
export BR2_DL_DIR=`pwd`/workspace/buildroot/dl
mkdir -p $BR2_DL_DIR
which arm-linux-gnueabihf-g++
make
