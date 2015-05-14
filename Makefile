 #
# $Id: Makefile 1253 2014-02-23 21:09:06Z ales.bardorfer $
#
# Red Pitaya OS/Ecosystem main Makefile
#
# Red Pitaya operating system (OS) consists of the following components:
# 1. boot.bin
#    a) First stage bootloader (FSBL)
#    b) U-boot
#    c) Optional fpga.bit
# 2. Linux kernel
# 3. Linux devicetree
# 4. Ramdisk with root filesystem
#
# There are many inter-relations between these components. This Makefile is
# responsible to build those in a coordinated way and to package them within
# the target redpitaya-OS ZIP archive.
#
# TODO #1: Make up a new name for OS dir, as OS is building one level higher now.

TMP = tmp

UBOOT_TAG = xilinx-v2015.1
LINUX_TAG = xilinx-v2015.1
DTREE_TAG = xilinx-v2015.1

UBOOT_DIR = $(TMP)/u-boot-xlnx-$(UBOOT_TAG)
LINUX_DIR = $(TMP)/linux-xlnx-$(LINUX_TAG)
DTREE_DIR = $(TMP)/device-tree-xlnx-$(DTREE_TAG)

UBOOT_TAR = $(TMP)/u-boot-xlnx-$(UBOOT_TAG).tar.gz
LINUX_TAR = $(TMP)/linux-xlnx-$(LINUX_TAG).tar.gz
DTREE_TAR = $(TMP)/device-tree-xlnx-$(DTREE_TAG).tar.gz

UBOOT_URL = https://github.com/Xilinx/u-boot-xlnx/archive/$(UBOOT_TAG).tar.gz
LINUX_URL = https://github.com/Xilinx/linux-xlnx/archive/$(LINUX_TAG).tar.gz
DTREE_URL = https://github.com/Xilinx/device-tree-xlnx/archive/$(DTREE_TAG).tar.gz

LINUX_CFLAGS = "-O2 -mtune=cortex-a9 -mfpu=neon -mfloat-abi=softfp"
UBOOT_CFLAGS = "-O2 -mtune=cortex-a9 -mfpu=neon -mfloat-abi=softfp"
ARMHF_CFLAGS = "-O2 -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard"

################################################################################
################################################################################

BUILD=build
TARGET=target
NAME=ecosystem

SOC_DIR=fpga

# targets
FPGA       = $(SOC_DIR)/out/red_pitaya.bit
FSBL       = $(SOC_DIR)/sdk/fsbl/executable.elf
MEMTEST    = $(SOC_DIR)/sdk/memtest/executable.elf
DTS        = $(SOC_DIR)/sdk/dts/system.dts
DEVICETREE = $(TMP)/devicetree.dtb
UBOOT      = $(TMP)/u-boot.elf
BOOT       = $(TMP)/boot.bin
TESTBOOT   = $(TMP)/testboot.bin
LINUX      = $(TMP)/uImage

URAMDISK_DIR    = OS/buildroot
NGINX_DIR       = Bazaar/nginx
MONITOR_DIR     = Test/monitor
GENERATE_DIR    = Test/generate
ACQUIRE_DIR     = Test/acquire
CALIB_DIR       = Test/calib
DISCOVERY_DIR   = OS/discovery
ECOSYSTEM_DIR   = Applications/ecosystem
SCPI_SERVER_DIR = scpi-server/
LIBRP_DIR       = api-mockup/rpbase/src
SDK_DIR         = SDK/
EXAMPLES_COMMUNICATION_DIR=Examples/Communication/C

URAMDISK=$(BUILD)/uramdisk.image.gz
NGINX=$(BUILD)/sbin/nginx
MONITOR=$(BUILD)/bin/monitor
GENERATE=$(BUILD)/bin/generate
ACQUIRE=$(BUILD)/bin/acquire
CALIB=$(BUILD)/bin/calib
DISCOVERY=$(BUILD)/sbin/discovery
ECOSYSTEM=$(BUILD)/www/apps/info/info.json
SCPI_SERVER = $(BUILD)/bin/scpi-server
LIBRP = $(BUILD)/lib/librp.so
GDBSERVER  = $(BUILD)/bin/gdbserver

# Versioning system
BUILD_NUMBER ?= 0
REVISION ?= devbuild
VER:=$(shell cat $(ECOSYSTEM_DIR)/info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')
VERSION=$(VER)-$(BUILD_NUMBER)
export BUILD_NUMBER
export REVISION
export VERSION

################################################################################
# external sources
################################################################################

all: zip

$(TARGET): $(BOOT) $(TESTBOOT) $(LINUX) $(DEVICETREE) $(URAMDISK) $(NGINX) $(MONITOR) $(GENERATE) $(ACQUIRE) $(CALIB) $(DISCOVERY) $(ECOSYSTEM) $(SCPI_SERVER) $(LIBRP) $(GDBSERVER) sdk rp_communication
	mkdir $(TARGET)
	cp $(BOOT) $(TARGET)
	cp $(TESTBOOT) $(TARGET)
	cp $(DEVICETREE) $(TARGET)
	cp $(LINUX) $(TARGET)
	cp -r $(BUILD)/* $(TARGET)
	cp -r OS/filesystem/* $(TARGET)
	rm `find $(TARGET) -iname .svn` -rf
	echo "" > $(TARGET)/version.txt
	echo "Red Pitaya GNU/Linux/Ecosystem version $(VERSION)" >> $(TARGET)/version.txt
	echo "" >> $(TARGET)/version.txt

$(BUILD):
	mkdir $(BUILD)

################################################################################
# FPGA build provides: $(FSBL), $(FPGA), $(DEVICETREE).
################################################################################

$(FPGA):
	make -C $(SOC_DIR)

$(FSBL): $(FPGA)

$(MEMTEST): $(FPGA)

################################################################################
# U-Boot build provides: $(UBOOT)
################################################################################

$(UBOOT_TAR):
	mkdir -p $(@D)
	curl -L $(UBOOT_URL) -o $@

$(UBOOT_DIR): $(UBOOT_TAR)
	mkdir -p $@
	tar -zxf $< --strip-components=1 --directory=$@
	patch -d tmp -p 0 < patches/u-boot-xlnx-$(UBOOT_TAG).patch
	cp patches/zynq_red_pitaya_defconfig $@/configs
	cp patches/zynq-red-pitaya.dts $@/arch/arm/dts
	cp patches/zynq_red_pitaya.h $@/include/configs
	cp patches/u-boot-lantiq.c $@/drivers/net/phy/lantiq.c

$(UBOOT): $(UBOOT_DIR)
	mkdir -p $(@D)
	make -C $< arch=ARM zynq_red_pitaya_defconfig
	make -C $< arch=ARM CFLAGS=$(UBOOT_CFLAGS) CROSS_COMPILE=arm-xilinx-linux-gnueabi- all
	cp $</u-boot $@

fw_printenv: $(UBOOT_DIR) $(UBOOT)
	make -C $< arch=ARM CFLAGS=$(ARMHF_CFLAGS) CROSS_COMPILE=arm-linux-gnueabihf- env
	cp $</tools/env/fw_printenv $@

################################################################################
# Linux build provides: $(LINUX)
################################################################################

$(LINUX_TAR):
	mkdir -p $(@D)
	curl -L $(LINUX_URL) -o $@

$(LINUX_DIR): $(LINUX_TAR)
	mkdir -p $@
	tar -zxf $< --strip-components=1 --directory=$@
	patch -d tmp -p 0 < patches/linux-xlnx-$(LINUX_TAG).patch
	cp patches/linux-lantiq.c $@/drivers/net/phy/lantiq.c

$(LINUX): $(LINUX_DIR)
	make -C $< mrproper
	make -C $< ARCH=arm xilinx_zynq_defconfig
	make -C $< ARCH=arm CFLAGS=$(LINUX_CFLAGS) \
	  -j $(shell grep -c ^processor /proc/cpuinfo) \
	  CROSS_COMPILE=arm-xilinx-linux-gnueabi- UIMAGE_LOADADDR=0x8000 uImage
	cp $</arch/arm/boot/uImage $@

################################################################################
# device tree processing
# TODO: here separate device trees should be provided for Ubuntu and buildroot
################################################################################

$(DEVICETREE): $(LINUX_DIR) $(FPGA) $(DTS)
	cp $(DTS) $(TMP)/devicetree.dts
	patch $(TMP)/devicetree.dts patches/devicetree.patch
	$(LINUX_DIR)/scripts/dtc/dtc -I dts -O dtb -o $(DEVICETREE) -i $(SOC_DIR)/sdk/dts/ $(TMP)/devicetree.dts

################################################################################
# boot file generator
################################################################################

$(BOOT): $(FSBL) $(FPGA) $(UBOOT)
	@echo img:{[bootloader] $(FSBL) $(FPGA) $(UBOOT) } > boot.bif
	bootgen -image boot.bif -w -o i $@

$(TESTBOOT): $(MEMTEST) $(FPGA) $(UBOOT)
	@echo img:{[bootloader] $(MEMTEST) $(FPGA) $(UBOOT) } > testboot.bif
	bootgen -image testboot.bif -w -o i $@

################################################################################
################################################################################

$(URAMDISK): $(BUILD)
	$(MAKE) -C $(URAMDISK_DIR)
	$(MAKE) -C $(URAMDISK_DIR) install INSTALL_DIR=$(abspath $(BUILD))

$(NGINX): $(URAMDISK)
	$(MAKE) -C $(NGINX_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	$(MAKE) -C $(NGINX_DIR) install DESTDIR=$(abspath $(BUILD))

$(MONITOR):
	$(MAKE) -C $(MONITOR_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	$(MAKE) -C $(MONITOR_DIR) install INSTALL_DIR=$(abspath $(BUILD))

$(GENERATE):
	$(MAKE) -C $(GENERATE_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	$(MAKE) -C $(GENERATE_DIR) install INSTALL_DIR=$(abspath $(BUILD))

$(ACQUIRE):
	$(MAKE) -C $(ACQUIRE_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	$(MAKE) -C $(ACQUIRE_DIR) install INSTALL_DIR=$(abspath $(BUILD))

$(CALIB):
	$(MAKE) -C $(CALIB_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	$(MAKE) -C $(CALIB_DIR) install INSTALL_DIR=$(abspath $(BUILD))

$(DISCOVERY):
	$(MAKE) -C $(DISCOVERY_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	$(MAKE) -C $(DISCOVERY_DIR) install INSTALL_DIR=$(abspath $(BUILD))

$(ECOSYSTEM):
	$(MAKE) -C $(ECOSYSTEM_DIR) install INSTALL_DIR=$(abspath $(BUILD))

$(SCPI_SERVER):
	$(MAKE) -C $(SCPI_SERVER_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	$(MAKE) -C $(SCPI_SERVER_DIR) install INSTALL_DIR=$(abspath $(BUILD))

$(LIBRP):
	$(MAKE) -C $(LIBRP_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	$(MAKE) -C $(LIBRP_DIR) install INSTALL_DIR=$(abspath $(BUILD))	

#Gdb server for remote debugging
$(GDBSERVER): #TODO: This is a temporary solution
	cp Test/gdb-server/gdbserver $(abspath $(BUILD))/bin

sdk:
	$(MAKE) -C $(SDK_DIR) clean include

sdkPub:
	$(MAKE) -C $(SDK_DIR) zip

rp_communication:
	make -C $(EXAMPLES_COMMUNICATION_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-

zip: $(TARGET) $(SDK)
	cd $(TARGET); zip -r ../$(NAME)-$(VER)-$(BUILD_NUMBER)-$(REVISION).zip *

clean:
	make -C $(LINUX_DIR) clean
	make -C $(SOC_DIR) clean
	make -C $(UBOOT_DIR) clean
	make -C $(NGINX_DIR) clean	
	make -C $(MONITOR_DIR) clean
	make -C $(GENERATE_DIR) clean
	make -C $(ACQUIRE_DIR) clean
	make -C $(CALIB_DIR) clean
	make -C $(DISCOVERY_DIR) clean
	make -C $(SCPI_SERVER_DIR) clean
	make -C $(LIBRP_DIR) clean
	make -C $(SDK_DIR) clean
	make -C $(EXAMPLES_COMMUNICATION_DIR) clean
	rm $(BUILD) -rf
	rm $(TARGET) -rf
	$(RM) $(NAME)*.zip
