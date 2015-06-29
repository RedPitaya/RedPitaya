export CROSS_COMPILE=arm-xilinx-linux-gnueabi-
source /usr/local/xilinx/Vivado/2013.3/settings32.sh
export PATH=$PATH:/usr/local/xilinx/SDK/2013.3/gnu/arm/lin/bin
export PATH=$PATH:/home/user/work/RedPitaya-master/OS/u-boot/u-boot-xlnx/tools
make

