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
BUILDROOT_TAG = 2015.5

UBOOT_DIR = $(TMP)/u-boot-xlnx-$(UBOOT_TAG)
LINUX_DIR = $(TMP)/linux-xlnx-$(LINUX_TAG)
DTREE_DIR = $(TMP)/device-tree-xlnx-$(DTREE_TAG)
BUILDROOT_DIR = $(TMP)/buildroot-$(BUILDROOT_TAG)

UBOOT_TAR = $(TMP)/u-boot-xlnx-$(UBOOT_TAG).tar.gz
LINUX_TAR = $(TMP)/linux-xlnx-$(LINUX_TAG).tar.gz
DTREE_TAR = $(TMP)/device-tree-xlnx-$(DTREE_TAG).tar.gz
BUILDROOT_TAR = $(TMP)/buildroot-$(BUILDROOT_TAG).tar.gz

# it is possible to use an alternative download location (local) by setting environment variables
UBOOT_URL ?= https://github.com/Xilinx/u-boot-xlnx/archive/$(UBOOT_TAG).tar.gz
LINUX_URL ?= https://github.com/Xilinx/linux-xlnx/archive/$(LINUX_TAG).tar.gz
DTREE_URL ?= https://github.com/Xilinx/device-tree-xlnx/archive/$(DTREE_TAG).tar.gz
BUILDROOT_URL ?= http://buildroot.uclibc.org/downloads/buildroot-$(BUILDROOT_TAG).tar.gz

UBOOT_GIT ?= https://github.com/Xilinx/u-boot-xlnx.git
LINUX_GIT ?= https://github.com/Xilinx/linux-xlnx.git
DTREE_GIT ?= https://github.com/Xilinx/device-tree-xlnx.git
BUILDROOT_GIT ?= http://git.buildroot.net/git/buildroot.git

LINUX_CFLAGS = "-O2 -mtune=cortex-a9 -mfpu=neon -mfloat-abi=softfp"
UBOOT_CFLAGS = "-O2 -mtune=cortex-a9 -mfpu=neon -mfloat-abi=softfp"
ARMHF_CFLAGS = "-O2 -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard"

################################################################################
#
################################################################################

# TODO, using Linux kernel 3.18 (Xilinx version 2015.1), it should be possible to use overlayfs
INSTALL_DIR=build
TARGET=target
NAME=ecosystem

# directories
FPGA_DIR        = fpga

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
URAMDISK_DIR    = OS/buildroot

# targets
FPGA            = $(FPGA_DIR)/out/red_pitaya.bit
FSBL            = $(FPGA_DIR)/sdk/fsbl/executable.elf
MEMTEST         = $(FPGA_DIR)/sdk/memtest/executable.elf
DTS             = $(FPGA_DIR)/sdk/dts/system.dts
DEVICETREE      = $(TMP)/devicetree.dtb
UBOOT           = $(TMP)/u-boot.elf
LINUX           = $(TMP)/uImage
BOOT            = $(TMP)/boot.bin
TESTBOOT        = $(TMP)/testboot.bin

NGINX           = $(INSTALL_DIR)/sbin/nginx
IDGEN           = $(INSTALL_DIR)/sbin/idgen
MONITOR         = $(INSTALL_DIR)/bin/monitor
GENERATE        = $(INSTALL_DIR)/bin/generate
ACQUIRE         = $(INSTALL_DIR)/bin/acquire
CALIB           = $(INSTALL_DIR)/bin/calib
DISCOVERY       = $(INSTALL_DIR)/sbin/discovery
ECOSYSTEM       = $(INSTALL_DIR)/www/apps/info/info.json
SCPI_SERVER     = $(INSTALL_DIR)/bin/scpi-server
LIBRP           = $(INSTALL_DIR)/lib/librp.so
LIBRPAPP        = $(INSTALL_DIR)/lib/librpapp.so
GDBSERVER       = $(INSTALL_DIR)/bin/gdbserver
LIBREDPITAYA    = shared/libredpitaya/libredpitaya.a

ENVTOOLS_CFG    = $(INSTALL_DIR)/etc/fw_env.config

UBOOT_SCRIPT_BUILDROOT = patches/u-boot.script.buildroot
UBOOT_SCRIPT_DEBIAN    = patches/u-boot.script.debian
UBOOT_SCRIPT           = $(INSTALL_DIR)/u-boot.scr

URAMDISK        = $(INSTALL_DIR)/uramdisk.image.gz


APP_SCOPE_DIR   = Applications/scope
APP_SCOPE       = $(INSTALL_DIR)/www/apps/scope

APP_SPECTRUM_DIR = Applications/spectrum-new
APP_SPECTRUM     = $(INSTALL_DIR)/www/apps/spectrum

################################################################################
# Versioning system
################################################################################

BUILD_NUMBER ?= 0
REVISION ?= devbuild
VER := $(shell cat $(ECOSYSTEM_DIR)/info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')
VERSION = $(VER)-$(BUILD_NUMBER)
export BUILD_NUMBER
export REVISION
export VERSION

################################################################################
# directories
################################################################################

################################################################################
# external sources
################################################################################

all: zip

$(TMP):
	mkdir -p $@

$(TARGET): $(BOOT) $(TESTBOOT) $(UBOOT_SCRIPT) $(DEVICETREE) $(LINUX) $(URAMDISK) $(NGINX) $(IDGEN) $(MONITOR) $(GENERATE) $(ACQUIRE) $(CALIB) $(DISCOVERY) $(ECOSYSTEM) $(SCPI_SERVER) $(LIBRP) $(LIBRPAPP) $(GDBSERVER) $(APP_SCOPE) $(APP_SPECTRUM) sdk rp_communication
	mkdir $(TARGET)
	cp $(BOOT)             $(TARGET)
	cp $(TESTBOOT)         $(TARGET)
	cp $(DEVICETREE)       $(TARGET)
	cp $(LINUX)            $(TARGET)
	cp -r $(INSTALL_DIR)/* $(TARGET)
	cp -r OS/filesystem/*  $(TARGET)
	echo "Red Pitaya GNU/Linux/Ecosystem version $(VERSION)" > $(TARGET)/version.txt

################################################################################
# FPGA build provides: $(FSBL), $(FPGA), $(DEVICETREE).
################################################################################

$(FPGA): $(DTREE_DIR)
	make -C $(FPGA_DIR)

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
	patch -d $@ -p 1 < patches/u-boot-xlnx-$(UBOOT_TAG).patch

$(UBOOT): $(UBOOT_DIR)
	mkdir -p $(@D)
	make -C $< arch=ARM zynq_red_pitaya_defconfig
	make -C $< arch=ARM CFLAGS=$(UBOOT_CFLAGS) all
	cp $</u-boot $@

$(UBOOT_SCRIPT): $(INSTALL_DIR) $(UBOOT_DIR) $(UBOOT_SCRIPT_BUILDROOT) $(UBOOT_SCRIPT_DEBIAN)
	$(UBOOT_DIR)/tools/mkimage -A ARM -O linux -T script -C none -a 0 -e 0 -n "boot Buildroot" -d $(UBOOT_SCRIPT_BUILDROOT) $@.buildroot
	$(UBOOT_DIR)/tools/mkimage -A ARM -O linux -T script -C none -a 0 -e 0 -n "boot Debian"    -d $(UBOOT_SCRIPT_DEBIAN)    $@.debian
	cp $@.buildroot $@

$(ENVTOOLS_CFG): $(UBOOT_DIR)
	mkdir -p $(INSTALL_DIR)/etc/
	cp $</tools/env/fw_env.config $(INSTALL_DIR)/etc

################################################################################
# Linux build provides: $(LINUX)
################################################################################

$(LINUX_TAR):
	mkdir -p $(@D)
	curl -L $(LINUX_URL) -o $@

$(LINUX_DIR): $(LINUX_TAR)
	mkdir -p $@
	tar -zxf $< --strip-components=1 --directory=$@
	patch -d $@ -p 1 < patches/linux-xlnx-$(LINUX_TAG).patch
	cp patches/linux-lantiq.c $@/drivers/net/phy/lantiq.c

$(LINUX): $(LINUX_DIR)
	make -C $< mrproper
	make -C $< ARCH=arm xilinx_zynq_defconfig
	make -C $< ARCH=arm CFLAGS=$(LINUX_CFLAGS) -j $(shell grep -c ^processor /proc/cpuinfo) UIMAGE_LOADADDR=0x8000 uImage
	cp $</arch/arm/boot/uImage $@

################################################################################
# device tree processing
# TODO: here separate device trees should be provided for Ubuntu and buildroot
################################################################################

$(DTREE_TAR):
	mkdir -p $(@D)
	curl -L $(DTREE_URL) -o $@

$(DTREE_DIR): $(DTREE_TAR)
	mkdir -p $@
	tar -zxf $< --strip-components=1 --directory=$@

$(DEVICETREE): $(DTREE_DIR) $(LINUX) $(FPGA) $(DTS)
	cp $(DTS) $(TMP)/devicetree.dts
	patch $(TMP)/devicetree.dts patches/devicetree.patch
	$(LINUX_DIR)/scripts/dtc/dtc -I dts -O dtb -o $(DEVICETREE) -i $(FPGA_DIR)/sdk/dts/ $(TMP)/devicetree.dts

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
# root file system
################################################################################

$(INSTALL_DIR):
	mkdir $(INSTALL_DIR)

$(URAMDISK): $(INSTALL_DIR)
	$(MAKE) -C $(URAMDISK_DIR)
	$(MAKE) -C $(URAMDISK_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

################################################################################
# API libraries
################################################################################

$(LIBREDPITAYA):
	$(MAKE) -C shared

$(LIBRP):
	$(MAKE) -C $(LIBRP_DIR)
	$(MAKE) -C $(LIBRP_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

$(LIBRPAPP):
	$(MAKE) -C $(LIBRPAPP_DIR)
	$(MAKE) -C $(LIBRPAPP_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

################################################################################
# Red Pitaya ecosystem
################################################################################

WEBSOCKETPP_TAG = 0.5.0
LUANGINX_TAG    = v0.8.7
NGINX_TAG       = 1.5.3

WEBSOCKETPP_URL = https://github.com/zaphoyd/websocketpp/archive/$(WEBSOCKETPP_TAG).tar.gz
CRYPTOPP_URL    = http://www.cryptopp.com/cryptopp562.zip
LIBJSON_URL     = http://sourceforge.net/projects/libjson/files/libjson_7.6.1.zip
LUANGINX_URL    = https://codeload.github.com/openresty/lua-nginx-module/tar.gz/$(LUANGINX_TAG)
NGINX_URL       = http://nginx.org/download/nginx-$(NGINX_TAG).tar.gz

WEBSOCKETPP_TAR = $(TMP)/websocketpp-$(WEBSOCKETPP_TAG).tar.gz
CRYPTOPP_TAR    = $(TMP)/cryptopp562.zip
LIBJSON_TAR     = $(TMP)/libjson_7.6.1.zip
LUANGINX_TAR    = $(TMP)/lua-nginx-module-$(LUANGINX_TAG).tr.gz
NGINX_TAR       = $(TMP)/nginx-$(NGINX_TAG).tar.gz

WEBSOCKETPP_DIR = Bazaar/nginx/ngx_ext_modules/ws_server/websocketpp
CRYPTOPP_DIR    = Bazaar/tools/cryptopp
LIBJSON_DIR     = Bazaar/tools/libjson
LUANGINX_DIR    = Bazaar/nginx/ngx_ext_modules/lua-nginx-module
NGINX_DIR       = Bazaar/nginx/nginx-1.5.3
BOOST_DIR       = Bazaar/nginx/ngx_ext_modules/ws_server/boost

$(WEBSOCKETPP_TAR):
	mkdir -p $(@D)
	curl -L $(WEBSOCKETPP_URL) -o $@

$(WEBSOCKETPP_DIR): $(WEBSOCKETPP_TAR)
	mkdir -p $@
	tar -zxf $< --strip-components=1 --directory=$@

$(CRYPTOPP_TAR):
	mkdir -p $(@D)
	curl -L $(CRYPTOPP_URL) -o $@

$(CRYPTOPP_DIR): $(CRYPTOPP_TAR)
	mkdir -p $@
	unzip $< -d $@
	patch -d $@ -p1 < patches/cryptopp.patch

$(LIBJSON_TAR):
	mkdir -p $(@D)
	curl -L $(LIBJSON_URL) -o $@

$(LIBJSON_DIR): $(LIBJSON_TAR)
	mkdir -p $@
	unzip $< -d $(@D)
	patch -d $@ -p1 < patches/libjson.patch

$(LUANGINX_TAR):
	mkdir -p $(@D)
	curl -L $(LUANGINX_URL) -o $@

$(LUANGINX_DIR): $(LUANGINX_TAR)
	mkdir -p $@
	tar -zxf $< --strip-components=1 --directory=$@
	patch -d $@ -p1 < patches/lua-nginx-module.patch

$(NGINX_TAR):
	mkdir -p $(@D)
	curl -L $(NGINX_URL) -o $@

$(NGINX_DIR): $(NGINX_TAR)
	mkdir -p $@
	tar -zxf $< --strip-components=1 --directory=$@
	patch -d $@ -p1 < patches/nginx.patch

$(BOOST_DIR): $(URAMDISK)
	ln -sf ../../../../OS/buildroot/buildroot-2014.02/output/build/boost-1.55.0 $<

$(NGINX): $(URAMDISK) $(LIBREDPITAYA) $(URAMDISK)
	$(MAKE) -C $(NGINX_DIR)
	$(MAKE) -C $(NGINX_DIR) install DESTDIR=$(abspath $(INSTALL_DIR))

$(IDGEN):
	$(MAKE) -C $(IDGEN_DIR)
	$(MAKE) -C $(IDGEN_DIR) install DESTDIR=$(abspath $(INSTALL_DIR))
	
################################################################################
# Red Pitaya tools
################################################################################

$(MONITOR):
	$(MAKE) -C $(MONITOR_DIR)
	$(MAKE) -C $(MONITOR_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

$(GENERATE):
	$(MAKE) -C $(GENERATE_DIR)
	$(MAKE) -C $(GENERATE_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

$(ACQUIRE):
	$(MAKE) -C $(ACQUIRE_DIR)
	$(MAKE) -C $(ACQUIRE_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

$(CALIB):
	$(MAKE) -C $(CALIB_DIR)
	$(MAKE) -C $(CALIB_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

$(DISCOVERY): $(URAMDISK) $(LIBREDPITAYA)
	$(MAKE) -C $(DISCOVERY_DIR)
	$(MAKE) -C $(DISCOVERY_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

$(ECOSYSTEM):
	$(MAKE) -C $(ECOSYSTEM_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

$(SCPI_SERVER): $(LIBRP) $(LIBRPAPP)
	$(MAKE) -C $(SCPI_SERVER_DIR)
	$(MAKE) -C $(SCPI_SERVER_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

################################################################################
# Red Pitaya applications
################################################################################

$(APP_SCOPE): $(LIBRP) $(LIBRPAPP) $(NGINX)
	$(MAKE) -C $(APP_SCOPE_DIR)
	$(MAKE) -C $(APP_SCOPE_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

$(APP_SPECTRUM): $(LIBRP) $(LIBRPAPP) $(NGINX)
	$(MAKE) -C $(APP_SPECTRUM_DIR)
	$(MAKE) -C $(APP_SPECTRUM_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

# Gdb server for remote debugging
# TODO: This is a temporary solution
$(GDBSERVER):
	cp Test/gdb-server/gdbserver $(abspath $(INSTALL_DIR))/bin

sdk:
	$(MAKE) -C $(SDK_DIR) clean include

sdkPub:
	$(MAKE) -C $(SDK_DIR) zip

rp_communication:
	make -C $(EXAMPLES_COMMUNICATION_DIR)

zip: $(TARGET) $(SDK)
	cd $(TARGET); zip -r ../$(NAME)-$(VER)-$(BUILD_NUMBER)-$(REVISION).zip *

clean:
	make -C $(LINUX_DIR) clean
	make -C $(FPGA_DIR) clean
	make -C $(UBOOT_DIR) clean
	make -C shared clean
	# todo, remove downloaded libraries and symlinks
	rm -rf Bazaar/tools/cryptopp
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
