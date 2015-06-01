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
# TODO, using Linux kernel 3.18 (Xilinx version 2015.1), it should be possible to use overlayfs
#INSTALL_DIR=$(BUILD)/opt/redpitaya
INSTALL_DIR=$(BUILD)/redpitaya
TARGET=target
NAME=ecosystem

LINUX_DIR       = OS/linux
LINUX_SOURCE_DIR= $(LINUX_DIR)/linux-xlnx
UBOOT_DIR       = OS/u-boot
SOC_DIR         = FPGA
URAMDISK_DIR    = OS/buildroot
NGINX_DIR       = Bazaar/nginx
IDGEN_DIR       = Bazaar/tools/idgen
MONITOR_DIR     = Test/monitor
GENERATE_DIR    = Test/generate
ACQUIRE_DIR     = Test/acquire
CALIB_DIR       = Test/calib
DISCOVERY_DIR   = OS/discovery
ECOSYSTEM_DIR   = Applications/ecosystem
SCPI_SERVER_DIR = scpi-server/
LIBRP_DIR       = api-mockup/rpbase
LIBRPAPP_DIR    = api-mockup/rpApplications
SDK_DIR         = SDK/
EXAMPLES_COMMUNICATION_DIR=Examples/Communication/C

LINUX=$(BUILD)/uImage
DEVICETREE=$(BUILD)/devicetree.dtb
UBOOT=$(BUILD)/u-boot.elf
FPGA=$(BUILD)/fpga.bit
FSBL=$(BUILD)/fsbl.elf
BOOT=$(BUILD)/boot.bin
TESTBOOT=testboot.bin
MEMTEST=$(BUILD)/memtest.elf
URAMDISK=$(BUILD)/uramdisk.image.gz

LIBREDPITAYA=shared/libredpitaya/libredpitaya.a

NGINX       = $(INSTALL_DIR)/sbin/nginx
IDGEN       = $(INSTALL_DIR)/sbin/idgen
MONITOR     = $(INSTALL_DIR)/bin/monitor
GENERATE    = $(INSTALL_DIR)/bin/generate
ACQUIRE     = $(INSTALL_DIR)/bin/acquire
CALIB       = $(INSTALL_DIR)/bin/calib
DISCOVERY   = $(INSTALL_DIR)/sbin/discovery
ECOSYSTEM   = $(INSTALL_DIR)/www/apps/info/info.json
SCPI_SERVER = $(INSTALL_DIR)/bin/scpi-server
LIBRP       = $(INSTALL_DIR)/lib/librp.so
LIBRPAPP    = $(INSTALL_DIR)/lib/librpapp.so
GDBSERVER   = $(INSTALL_DIR)/bin/gdbserver

APP_SCOPE_DIR = Applications/scope
APP_SCOPE     = $(INSTALL_DIR)/www/apps/scope

APP_SPECTRUM_DIR = Applications/spectrum-new
APP_SPECTRUM     = $(INSTALL_DIR)/www/apps/spectrum

# Versioning system
BUILD_NUMBER ?= 0
REVISION ?= devbuild
VER:=$(shell cat $(ECOSYSTEM_DIR)/info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')
VERSION=$(VER)-$(BUILD_NUMBER)
export BUILD_NUMBER
export REVISION
export VERSION

all: zip

$(TARGET): $(BOOT) $(TESTBOOT) $(LINUX) $(DEVICETREE) $(URAMDISK) $(NGINX) $(IDGEN) $(MONITOR) $(GENERATE) $(ACQUIRE) $(CALIB) $(DISCOVERY) $(ECOSYSTEM) $(SCPI_SERVER) $(LIBRP) $(LIBRPAPP) $(GDBSERVER) $(APP_SCOPE) $(APP_SPECTRUM) sdk rp_communication
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

$(INSTALL_DIR):
	mkdir $(INSTALL_DIR)


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

$(LIBREDPITAYA):
	$(MAKE) -C shared CROSS_COMPILE=arm-xilinx-linux-gnueabi-

$(NGINX): $(URAMDISK) $(LIBREDPITAYA)
	$(MAKE) -C $(NGINX_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	$(MAKE) -C $(NGINX_DIR) install DESTDIR=$(abspath $(INSTALL_DIR))

$(IDGEN):
	$(MAKE) -C $(IDGEN_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	$(MAKE) -C $(IDGEN_DIR) install DESTDIR=$(abspath $(INSTALL_DIR))
	
$(MONITOR):
	$(MAKE) -C $(MONITOR_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	$(MAKE) -C $(MONITOR_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

$(GENERATE):
	$(MAKE) -C $(GENERATE_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	$(MAKE) -C $(GENERATE_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

$(ACQUIRE):
	$(MAKE) -C $(ACQUIRE_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	$(MAKE) -C $(ACQUIRE_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

$(CALIB):
	$(MAKE) -C $(CALIB_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	$(MAKE) -C $(CALIB_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

$(DISCOVERY): $(URAMDISK) $(LIBREDPITAYA)
	$(MAKE) -C $(DISCOVERY_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	$(MAKE) -C $(DISCOVERY_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

$(ECOSYSTEM):
	$(MAKE) -C $(ECOSYSTEM_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

$(SCPI_SERVER): $(LIBRP) $(LIBRPAPP)
	$(MAKE) -C $(SCPI_SERVER_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	$(MAKE) -C $(SCPI_SERVER_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

$(LIBRP):
	$(MAKE) -C $(LIBRP_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	$(MAKE) -C $(LIBRP_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

$(LIBRPAPP):
	$(MAKE) -C $(LIBRPAPP_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	$(MAKE) -C $(LIBRPAPP_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

$(APP_SCOPE): $(LIBRP) $(LIBRPAPP) $(NGINX)
	$(MAKE) -C $(APP_SCOPE_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	$(MAKE) -C $(APP_SCOPE_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

$(APP_SPECTRUM): $(LIBRP) $(LIBRPAPP) $(NGINX)
	$(MAKE) -C $(APP_SPECTRUM_DIR) CROSS_COMPILE=arm-xilinx-linux-gnueabi-
	$(MAKE) -C $(APP_SPECTRUM_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

#Gdb server for remote debugging
$(GDBSERVER): #TODO: This is a temporary solution
	cp Test/gdb-server/gdbserver $(abspath $(INSTALL_DIR))/bin

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
	make -C shared clean
	make -C $(NGINX_DIR) clean	
	make -C $(MONITOR_DIR) clean
	make -C $(GENERATE_DIR) clean
	make -C $(ACQUIRE_DIR) clean
	make -C $(CALIB_DIR) clean
	make -C $(DISCOVERY_DIR) clean
	make -C $(SCPI_SERVER_DIR) clean
	make -C $(LIBRP_DIR)    clean
	make -C $(LIBRPAPP_DIR) clean
	make -C $(SDK_DIR) clean
	make -C $(EXAMPLES_COMMUNICATION_DIR) clean
	rm $(BUILD) -rf
	rm $(TARGET) -rf
	$(RM) $(NAME)*.zip
