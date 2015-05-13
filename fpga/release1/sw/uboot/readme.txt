


### Get U-boot code

git clone git://github.com/Xilinx/u-boot-xlnx.git
cd u-boot-xlnx

git checkout 0f6dbff16b792a106f52ca37f4503335af30601b


### Patch code

patch -p1 < ../u-boot_patch.txt




### Build

export CROSS_COMPILE=arm-xilinx-linux-gnueabi-
export PATH=/opt/CodeSourcery/2012.09/bin/:$PATH

export BUILD_DIR=$PWD/build
make arch=ARM zynq_zed




### Rename ./build/u-boot to u-boot.elf and use it

