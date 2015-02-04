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


BUILD=build
TARGET=target
NAME=ecosystem

LINUX_DIR=OS/linux
LINUX_SOURCE_DIR=$(LINUX_DIR)/linux-xlnx
UBOOT_DIR=OS/u-boot
SOC_DIR=FPGA
URAMDISK_DIR=OS/buildroot
NGINX_DIR=Bazaar/nginx
MONITOR_DIR=Test/monitor
GENERATE_DIR=Test/generate
ACQUIRE_DIR=Test/acquire
CALIB_DIR=Test/calib
DISCOVERY_DIR=OS/discovery
ECOSYSTEM_DIR=Applications/ecosystem
SCPI_SERVER_DIR=scpi-server/
RPLIB_DIR=api-mockup/rpbase/src
SDK_DIR=SDK/

LINUX=$(BUILD)/uImage
DEVICETREE=$(BUILD)/devicetree.dtb
UBOOT=$(BUILD)/u-boot.elf
BOOT=$(BUILD)/boot.bin
FPGA=$(BUILD)/fpga.bit
FSBL=$(BUILD)/fsbl.elf
TESTBOOT=testboot.bin
MEMTEST=$(BUILD)/memtest.elf
URAMDISK=$(BUILD)/uramdisk.image.gz
NGINX=$(BUILD)/sbin/nginx
MONITOR=$(BUILD)/bin/monitor
GENERATE=$(BUILD)/bin/generate
ACQUIRE=$(BUILD)/bin/acquire
CALIB=$(BUILD)/bin/calib
DISCOVERY=$(BUILD)/sbin/discovery
ECOSYSTEM=$(BUILD)/www/apps/info/info.json
SCPI_SERVER = $(BUILD)/bin/scpi-server
RPLIB = $(BUILD)/lib/librp.so
GDBSERVER  = $(BUILD)/bin/gdbserver

# Versioning system
BUILD_NUMBER ?= 0
REVISION ?= devbuild
VER:=$(shell cat $(ECOSYSTEM_DIR)/info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')
VERSION=$(VER)-$(BUILD_NUMBER)
export BUILD_NUMBER
export REVISION
export VERSION

all: zip

$(TARGET): $(BOOT) $(TESTBOOT) $(LINUX) $(DEVICETREE) $(URAMDISK) $(NGINX) $(MONITOR) $(GENERATE) $(ACQUIRE) $(CALIB) $(DISCOVERY) $(ECOSYSTEM) $(SCPI_SERVER) $(RPLIB) $(GDBSERVER) sdk
	mkdir $(TARGET)
	cp -r $(BUILD)/* $(TARGET)
	rm -f $(TARGET)/fsbl.elf $(TARGET)/fpga.bit $(TARGET)/u-boot.elf $(TARGET)/devicetree.dts $(TARGET)/memtest.elf
	cp -r OS/filesystem/* $(TARGET)
	rm `find $(TARGET) -iname .svn` -rf
	echo "" > $(TARGET)/version.txt
	echo "Red Pitaya GNU/Linux/Ecosystem version $(VERSION)" >> $(TARGET)/version.txt
	echo "" >> $(TARGET)/version.txt


$(BUILD):
	mkdir $(BUILD)


# Linux build provides: uImage kernel, dtc compiler.
$(LINUX): $(BUILD)
	make -C $(LINUX_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	make -C $(LINUX_DIR) install INSTALL_DIR=$(abspath $(BUILD))

# FPGA build provides: fsbl.elf, fpga.bit, devicetree.dts.
$(FPGA): $(BUILD)
	make -C $(SOC_DIR)
	make -C $(SOC_DIR) install INSTALL_DIR=$(abspath $(BUILD))

# U-Boot build provides: u-boot.elf
$(UBOOT): $(BUILD)
	make -C $(UBOOT_DIR) RP_VERSION=$(VERSION) RP_REVISION=$(REVISION)
	make -C $(UBOOT_DIR) install INSTALL_DIR=$(abspath $(BUILD))

$(BOOT): $(BUILD) $(UBOOT) $(FPGA)
	@echo img:{[bootloader] $(FSBL) $(FPGA) $(UBOOT) } > b.bif
	bootgen -image b.bif -w -o i $@

$(TESTBOOT): $(BOOT)
	@echo img:{[bootloader] $(MEMTEST) $(FPGA) $(UBOOT) } > b.bif
	bootgen -image b.bif -w -o i $@

$(DEVICETREE): $(BUILD) $(LINUX) $(FPGA)
	$(LINUX_SOURCE_DIR)/scripts/dtc/dtc -I dts -O dtb -o $(DEVICETREE) $(BUILD)/devicetree.dts

$(URAMDISK): $(BUILD)
	$(MAKE) -C $(URAMDISK_DIR)
	$(MAKE) -C $(URAMDISK_DIR) install INSTALL_DIR=$(abspath $(BUILD))

$(NGINX): $(URAMDISK)
	$(MAKE) -C $(NGINX_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	$(MAKE) -C $(NGINX_DIR) install DESTDIR=$(abspath $(BUILD))

$(MONITOR):
	$(MAKE) -C $(MONITOR_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-

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

$(RPLIB):
	$(MAKE) -C $(RPLIB_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	$(MAKE) -C $(RPLIB_DIR) install INSTALL_DIR=$(abspath $(BUILD))	

#Gdb server for remote debugging
$(GDBSERVER): #TODO: This is a temporary solution
	cp Test/gdb-server/gdbserver $(abspath $(BUILD))/bin

#sdk:
	#$(MAKE) -C $(SDK_DIR)
	
zip: $(TARGET)
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
	make -C $(RPLIB_DIR) clean
	make -C $(SDK_DIR) clean
	rm $(BUILD) -rf
	rm $(TARGET) -rf
	$(RM) $(NAME)*.zip
