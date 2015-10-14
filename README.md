# Red Pitaya ecosystem and applications

Here you will find the sources of various software components of the
Red Pitaya system. The components are mainly contained in dedicated
directories, however, due to the nature of the Xilinx SoC "All 
Programmable" paradigm and the way several components are interrelated,
some components might be spread across many directories or found at
different places one would expect.

| directories  | contents
|--------------|----------------------------------------------------------------
| api          | `librp.so` API source code
| Applications | Red Pitaya applications (controller modules & GUI clients).
| apps-free    | Red Pitaya application for the old environment (also with controler modules & GUI clients).
| Bazaar       | Nginx server with dependencies, Red Pitaya Bazaar module & application controller module loader.
| fpga         | FPGA design for the inital set of Red Pitaya applications.
| OS/buildroot | GNU/Linux operating system components
| patches      | Directory containing red pitaya patches
| scpi-server  | Scpi server directory, containing red pitaya core scpi server
| Test         | Command line utilities (acquire, generate, ...).
| shared       | `libredpitaya.so` API source code (to be deprecated soon hopefully!)

## Requirements

Redpitaya is developed on Linux, so Linux (preferably Ubuntu) is also the only platform we support.

You will need the following to build the Red Pitaya components:
1. Various development packages:
```bash
sudo apt-get install make u-boot-tools curl xz-utils nano
```
2. Xilinx [Vivado 2015.2](http://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/vivado-design-tools/2015-2.html) FPGA development tools. The SDK (bare metal toolchain) must also be installed, be carefull during the install process to select it. Preferably use the default install location.
3. Linaro [ARM toolchain](https://releases.linaro.org/14.11/components/toolchain/binaries/arm-linux-gnueabihf/) for cross compiling Linux applications. We recommend to install it to `/opt/linaro/` since build process instructions relly on it.
```bash
TOOLCHAIN="http://releases.linaro.org/14.11/components/toolchain/binaries/arm-linux-gnueabihf/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf.tar.xz"
#TOOLCHAIN="http://releases.linaro.org/15.02/components/toolchain/binaries/arm-linux-gnueabihf/gcc-linaro-4.9-2015.02-3-x86_64_arm-linux-gnueabihf.tar.xz"
curl -O $TOOLCHAIN
sudo mkdir -p /opt/linaro
sudo chown $USER:$USER /opt/linaro
tar -xpf *linaro*.tar.xz -C /opt/linaro
```

# Build process

Go to your prefered development directory and clone the Red Pitaya repository from GitHub.
```bash
git clone https://github.com/RedPitaya/RedPitaya.git
cd RedPitaya
```

An example script `settings.sh` is provided for setting all necessary environment variables. The script assumes some default tool install paths, so it might need editing if install paths other than the ones described above were used.
```bash
. settings.sh
```

Prepare a download cache for various source tarballs. This is an optional step which will speadup the build process by avoiding downloads for all but the first build. There is a default cache path defined in the `settings.sh` script, you can edit it and avoid a rebuild the next time.
```bash
mkdir -p dl
export BR2_DL_DIR=$PWD/dl
```

To build everything just run `make`.
```bash
make
```

# Partial reguild process

The next components can be built separately.
- FPGA + device tree
- API
- free applications
- SCPI server
- Linux kernel
- Debian OS

## Base system

Here *base system* represents everything before Linux user space.

### FPGA and device tree

Detailed instructions are provided for [building the FPGA](fpga/README.md#build-process) including some [device tree details](fpga/README.md#device-tree).

### U-boot

To build the U-Boot binary and boot scripts (used to select between booting into Buildroot or Debian):
```bash
make tmp/u-boot.elf
make build/u-boot.scr
```
The build process downloads the Xilinx version of U-Boot sources from Github, applies patches and starts the build process. Patches are available in the `patches/` directory.

### Linux kernel

To build a Linux image:
```bash
make tmp/uImage
```
The build process downloads the Xilinx version of Linux sources from Github, applies patches and starts the build process. Patches are available in the `patches/` directory.

### Boot file

The created boot file contains FSBL, FPGA bitstream and U-Boot binary.
```bash
make tmp/boot.bin.uboot
```
Since file `tmp/boot.bin.uboot` is created it should be renamed to simply `tmp/boot.bin`. There are some preparations for creating a memory test `tmp/boot.bin.memtest` which would run from the SD card, but it did not go es easy es we would like, so it is not working.

## Linux user space

### Buildroot

Buildroot is the most basic Linux distribution available for Red Pitaya. It is also used to provide some sources which are dependencies for Userspace applications.
```bash
make build/uramdisk.image.gz
``` 

### Debian OS

[Debian OS instructions](OS/debian/README.md) are detailed elsewhere.

### API

Only instructions for the basic API are provided:
Navigate to the `api/rpbase` folder and run:
```bash
make
```
The output of this process is the Red Pitaya `librp.so` library in `api/lib` directory.

### Free applications

To build apps free, follow the instructions given at apps-free [README.md](apps-free/README.md) file.

### SCPI server

Scpi server README can be found [here](scpi-server/README.md)

