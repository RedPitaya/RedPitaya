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

ifndef CROSS_COMPILE
$(error Are you sure to compile w/o CROSS_COMPILE?  Please start settings.sh prior to this make command)
endif

# check, which boot image / file system should be entered from the FSBL (first stage boot loader)
ifndef FSBL_BR
ifndef FSBL_DEBIAN
$(error At least one of FSBL_BR or FSBL_DEBIAN has to be enabled for the build process. See in settings.sh for more details)
endif
endif

# check if download cache directory is available
ifdef BR2_DL_DIR
DL=$(BR2_DL_DIR)
else
DL=$(TMP)
endif
#TODO: DL = $(BR2_DL_DIR)

UBOOT_TAG     = xilinx-v2015.2
LINUX_TAG     = xilinx-v2015.1
DTREE_TAG     = xilinx-v2015.1
#BUILDROOT_TAG = 2015.5

UBOOT_DIR     = $(TMP)/u-boot-xlnx-$(UBOOT_TAG)
LINUX_DIR     = $(TMP)/linux-xlnx-$(LINUX_TAG)
DTREE_DIR     = $(TMP)/device-tree-xlnx-$(DTREE_TAG)
BUILDROOT_DIR = $(TMP)/buildroot-$(BUILDROOT_TAG)

UBOOT_TAR     = $(DL)/u-boot-xlnx-$(UBOOT_TAG).tar.gz
LINUX_TAR     = $(DL)/linux-xlnx-$(LINUX_TAG).tar.gz
DTREE_TAR     = $(DL)/device-tree-xlnx-$(DTREE_TAG).tar.gz
BUILDROOT_TAR = $(DL)/buildroot-$(BUILDROOT_TAG).tar.gz

# it is possible to use an alternative download location (local) by setting environment variables
UBOOT_URL     ?= https://github.com/Xilinx/u-boot-xlnx/archive/$(UBOOT_TAG).tar.gz
LINUX_URL     ?= https://github.com/Xilinx/linux-xlnx/archive/$(LINUX_TAG).tar.gz
DTREE_URL     ?= https://github.com/Xilinx/device-tree-xlnx/archive/$(DTREE_TAG).tar.gz
BUILDROOT_URL ?= http://buildroot.uclibc.org/downloads/buildroot-$(BUILDROOT_TAG).tar.gz

UBOOT_GIT     ?= https://github.com/Xilinx/u-boot-xlnx.git
LINUX_GIT     ?= https://github.com/Xilinx/linux-xlnx.git
DTREE_GIT     ?= https://github.com/Xilinx/device-tree-xlnx.git
BUILDROOT_GIT ?= http://git.buildroot.net/git/buildroot.git

ifeq ($(CROSS_COMPILE),arm-xilinx-linux-gnueabi-)
SYSROOT=$(PWD)/OS/buildroot/buildroot-2014.02/output/host/usr/arm-buildroot-linux-gnueabi/sysroot
LINUX_CFLAGS = "-O2 -mtune=cortex-a9 -mfpu=neon -mfloat-abi=soft"
UBOOT_CFLAGS = "-O2 -mtune=cortex-a9 -mfpu=neon -mfloat-abi=soft"
else
SYSROOT=$(PWD)/OS/buildroot/buildroot-2014.02/output/host/usr/arm-buildroot-linux-gnueabihf/sysroot
LINUX_CFLAGS = "-O2 -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard"
UBOOT_CFLAGS = "-O2 -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard"
endif

################################################################################
#
################################################################################

INSTALL_DIR=build
TARGET=target
NAME=ecosystem

# directories
FPGA_DIR        = fpga

NGINX_DIR       = Bazaar/nginx
IDGEN_DIR       = Bazaar/tools/idgen
OS_TOOLS_DIR    = OS/tools
ECOSYSTEM_DIR   = Applications/ecosystem
LIBRP_DIR       = api/rpbase
LIBRPAPP_DIR    = api/rpApplications
SDK_DIR         = SDK/
BR_BOOST_DIR    = OS/buildroot/buildroot-2014.02/output/build/boost-1.55.0

# targets
FPGA            = $(FPGA_DIR)/out/red_pitaya.bit
FSBL            = $(FPGA_DIR)/sdk/fsbl/executable.elf
MEMTEST         = $(FPGA_DIR)/sdk/dram_test/executable.elf
DTS             = $(FPGA_DIR)/sdk/dts/system.dts
DEVICETREE      = $(TMP)/devicetree.dtb
UBOOT           = $(TMP)/u-boot.elf
LINUX           = $(TMP)/uImage
BOOT_UBOOT      = $(TMP)/boot.bin

NGINX           = $(INSTALL_DIR)/sbin/nginx
IDGEN           = $(INSTALL_DIR)/sbin/idgen
DISCOVERY       = $(INSTALL_DIR)/sbin/discovery.sh
HEARTBEAT       = $(INSTALL_DIR)/sbin/heartbeat.sh

################################################################################
# Versioning system
################################################################################

BUILD_NUMBER ?= 0
REVISION ?= devbuild
VER := $(shell cat $(ECOSYSTEM_DIR)/info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')
GIT_BRANCH_LOCAL = $(shell echo $(GIT_BRANCH) | sed -e 's/.*\///')
VERSION = $(VER)-$(BUILD_NUMBER)-$(REVISION)
export BUILD_NUMBER
export REVISION
export VERSION

define GREET_MSG
##############################################################################
# Red Pitaya GNU/Linux Ecosystem
# Version: $(VER)
# Branch: $(GIT_BRANCH_LOCAL)
# Build: $(BUILD_NUMBER)
# Commit: $(GIT_COMMIT)
##############################################################################
endef
export GREET_MSG

################################################################################
# tarball
################################################################################

.PHONY: target_base

all: zip sdk apps-free

$(TMP):
	mkdir -p $@

ifdef FSBL_BR
ifdef FSBL_DEBIAN
$(info *** make BuildRoot and build for Debian entry ***) 
$(TARGET): $(BOOT_UBOOT) u-boot $(DEVICETREE) $(LINUX) buildroot $(IDGEN) $(NGINX) \
	   examples $(DISCOVERY) $(HEARTBEAT) ecosystem \
	   scpi api apps_pro rp_communication \
	   target_base
else
$(info *** make BuildRoot ***)
$(TARGET): $(BOOT_UBOOT) u-boot $(DEVICETREE) $(LINUX) buildroot $(IDGEN) $(NGINX) \
	   examples $(DISCOVERY) $(HEARTBEAT) ecosystem \
	   scpi api apps_pro rp_communication \
	   target_base
endif

else
$(info *** build for Debian entry ***)
$(TARGET): $(BOOT_UBOOT) u-boot $(DEVICETREE) $(LINUX) $(IDGEN) $(NGINX) \
	   examples $(DISCOVERY) $(HEARTBEAT) ecosystem \
	   scpi api apps_pro rp_communication \
	   target_base
endif

target_base:
	mkdir -p               $(TARGET)
	# copy boot images and select FSBL as default
	cp $(BOOT_UBOOT)       $(TARGET)/boot.bin
	# copy device tree and Linux kernel
	cp $(DEVICETREE)       $(TARGET)
	cp $(LINUX)            $(TARGET)
	# copy FPGA bitstream images and decompress them
	mkdir -p               $(TARGET)/fpga
	cp $(FPGA)             $(TARGET)/fpga/fpga.bit
	cp fpga/archive/*.xz   $(TARGET)/fpga
	cd $(TARGET)/fpga; xz -df *.xz
	#
	cp -r $(INSTALL_DIR)/* $(TARGET)
	cp -r OS/filesystem/*  $(TARGET)
	@echo "$$GREET_MSG" >  $(TARGET)/version.txt
	# copy configuration file for WiFi access point
	cp OS/debian/overlay/etc/hostapd/hostapd.conf $(TARGET)/hostapd.conf
	# copy static library that was used for compilation nginx and create shared object of it
	cp $(SYSROOT)/usr/lib/libstdc++.a $(TARGET)/lib; \
		$(CROSS_COMPILE)$(CXX) -shared -o $(TARGET)/lib/libstdc++.so \
		-Wl,--whole-archive -L $(TARGET)/usr/lib -lstdc++ -Wl,--no-whole-archive

zip: $(TARGET)
	cd $(TARGET); zip -r ../$(NAME)-$(VERSION).zip *

################################################################################
# FPGA build provides: $(FSBL), $(FPGA), $(DEVICETREE).
################################################################################

.PHONY: fpga

fpga: $(DTREE_DIR)
	make -C $(FPGA_DIR)

################################################################################
# U-Boot build provides: $(UBOOT)
################################################################################

ENVTOOLS_CFG    = $(INSTALL_DIR)/etc/fw_env.config

UBOOT_SCRIPT_BUILDROOT = patches/u-boot.script.buildroot
UBOOT_SCRIPT_DEBIAN    = patches/u-boot.script.debian
UBOOT_SCRIPT           = $(INSTALL_DIR)/u-boot.scr

.PHONY: u-boot

u-boot: $(UBOOT) $(UBOOT_SCRIPT) $(ENVTOOLS_CFG)

$(UBOOT_TAR): | $(DL)
	wget $(UBOOT_URL) -O $@

$(UBOOT_DIR): $(UBOOT_TAR)
	mkdir -p $@
	tar -zxf $< --strip-components=1 --directory=$@
	patch -d $@ -p 1 < patches/u-boot-xlnx-$(UBOOT_TAG).patch

$(UBOOT): $(UBOOT_DIR)
	mkdir -p $(@D)
	make -C $< ARCH=arm zynq_red_pitaya_defconfig
	make -C $< ARCH=arm CFLAGS=$(UBOOT_CFLAGS) all
	cp $</u-boot $@

$(UBOOT_SCRIPT): $(INSTALL_DIR) $(UBOOT_DIR) $(UBOOT_SCRIPT_BUILDROOT) $(UBOOT_SCRIPT_DEBIAN)
	$(UBOOT_DIR)/tools/mkimage -A ARM -O linux -T script -C none -a 0 -e 0 -n "boot Buildroot" -d $(UBOOT_SCRIPT_BUILDROOT) $@.buildroot
	$(UBOOT_DIR)/tools/mkimage -A ARM -O linux -T script -C none -a 0 -e 0 -n "boot Debian"    -d $(UBOOT_SCRIPT_DEBIAN)    $@.debian
ifdef FSBL_DEBIAN
	cp $@.debian $@
else
ifdef FSBL_BR
	cp $@.buildroot $@
endif
endif

$(ENVTOOLS_CFG): $(UBOOT_DIR)
	mkdir -p $(INSTALL_DIR)/etc/
	cp $</tools/env/fw_env.config $(INSTALL_DIR)/etc

################################################################################
# Linux build provides: $(LINUX)
################################################################################

$(LINUX_TAR): | $(DL)
	wget $(LINUX_URL) -O $@

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

$(DTREE_TAR): | $(DL)
	wget $(DTREE_URL) -O $@

$(DTREE_DIR): $(DTREE_TAR)
	mkdir -p $@
	tar -zxf $< --strip-components=1 --directory=$@

$(DEVICETREE): $(DTREE_DIR) $(LINUX) fpga
	cp $(DTS) $(TMP)/devicetree.dts
	patch $(TMP)/devicetree.dts patches/devicetree.patch
	$(LINUX_DIR)/scripts/dtc/dtc -I dts -O dtb -o $(DEVICETREE) -i $(FPGA_DIR)/sdk/dts/ $(TMP)/devicetree.dts

################################################################################
# boot file generator
################################################################################

$(BOOT_UBOOT): fpga $(UBOOT)
	@echo img:{[bootloader] $(FSBL) $(FPGA) $(UBOOT) } > boot_uboot.bif
	bootgen -image boot_uboot.bif -w -o $@

################################################################################
# root file system
################################################################################

URAMDISK_DIR    = OS/buildroot

.PHONY: buildroot buildroot_boost

$(INSTALL_DIR):
	mkdir $(INSTALL_DIR)

buildroot_boost: $(INSTALL_DIR)
	#$(MAKE) -C $(URAMDISK_DIR) boost   # at the moment there is no quick solution for a less time consuming way
	$(MAKE) -C $(URAMDISK_DIR)          # using full build instead

buildroot: $(INSTALL_DIR)
	$(MAKE) -C $(URAMDISK_DIR)
	$(MAKE) -C $(URAMDISK_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

################################################################################
# API libraries
################################################################################

.PHONY: api librp librpapp libredpitaya

libredpitaya:
	$(MAKE) -C shared

librp:
	$(MAKE) -C $(LIBRP_DIR)
	$(MAKE) -C $(LIBRP_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

ifdef ENABLE_LICENSING

api: librp librpapp

librpapp:
	$(MAKE) -C $(LIBRPAPP_DIR)
	$(MAKE) -C $(LIBRPAPP_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

else

api: librp

endif

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

WEBSOCKETPP_TAR = $(DL)/websocketpp-$(WEBSOCKETPP_TAG).tar.gz
CRYPTOPP_TAR    = $(DL)/cryptopp562.zip
LIBJSON_TAR     = $(DL)/libjson_7.6.1.zip
LUANGINX_TAR    = $(DL)/lua-nginx-module-$(LUANGINX_TAG).tr.gz
NGINX_TAR       = $(DL)/nginx-$(NGINX_TAG).tar.gz

WEBSOCKETPP_DIR = Bazaar/nginx/ngx_ext_modules/ws_server/websocketpp
CRYPTOPP_DIR    = Bazaar/tools/cryptopp
LIBJSON_DIR     = Bazaar/tools/libjson
LUANGINX_DIR    = Bazaar/nginx/ngx_ext_modules/lua-nginx-module
NGINX_SRC_DIR   = Bazaar/nginx/nginx-1.5.3
NGINX_BOOST_DIR = Bazaar/nginx/ngx_ext_modules/ws_server/boost

.PHONY: ecosystem nginx 

$(WEBSOCKETPP_TAR): | $(DL)
	wget $(WEBSOCKETPP_URL) -O $@

$(WEBSOCKETPP_DIR): $(WEBSOCKETPP_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@

$(CRYPTOPP_TAR): | $(DL)
	wget $(CRYPTOPP_URL) -O $@

$(CRYPTOPP_DIR): $(CRYPTOPP_TAR)
	mkdir -p $@
	unzip $< -d $@
	patch -d $@ -p1 < patches/cryptopp.patch

$(LIBJSON_TAR): | $(DL)
	wget $(LIBJSON_URL) -O $@

$(LIBJSON_DIR): $(LIBJSON_TAR)
	mkdir -p $@
	unzip $< -d $(@D)
	patch -d $@ -p1 < patches/libjson.patch

$(LUANGINX_TAR): | $(DL)
	wget $(LUANGINX_URL) -O $@

$(LUANGINX_DIR): $(LUANGINX_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@
	patch -d $@ -p1 < patches/lua-nginx-module.patch

$(NGINX_TAR): | $(DL)
	wget $(NGINX_URL) -O $@

$(NGINX_SRC_DIR): $(NGINX_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@
	patch -d $@ -p1 < patches/nginx.patch
	cp -f patches/nginx.conf $@/conf/

$(BR_BOOST_DIR): buildroot_boost

$(NGINX_BOOST_DIR): $(BR_BOOST_DIR)
	ln -sf ../../../../$< $@

$(NGINX): $(NGINX_BOOST_DIR) libredpitaya $(CRYPTOPP_DIR) $(LIBJSON_DIR) $(LUANGINX_DIR) $(NGINX_SRC_DIR) $(WEBSOCKETPP_DIR)
	$(MAKE) -C $(NGINX_DIR) SYSROOT=$(SYSROOT)
	$(MAKE) -C $(NGINX_DIR) install DESTDIR=$(abspath $(INSTALL_DIR))

$(IDGEN): $(NGINX)
	$(MAKE) -C $(IDGEN_DIR)
	$(MAKE) -C $(IDGEN_DIR) install DESTDIR=$(abspath $(INSTALL_DIR))

################################################################################
# SCPI server
################################################################################

#SCPI_PARSER_TAG = fbe83efc8183980109846bd884da28104ca1faa1
#SCPI_PARSER_URL = https://github.com/j123b567/scpi-parser/archive/$(SCPI_PARSER_TAG).tar.gz
SCPI_PARSER_TAG = fb6979d1926bb6813898012de934eca366d93ff8
SCPI_PARSER_URL = https://github.com/RedPitaya/scpi-parser/archive/$(SCPI_PARSER_TAG).tar.gz
SCPI_PARSER_TAR = $(DL)/scpi-parser-$(SCPI_PARSER_TAG).tar.gz
SCPI_SERVER_DIR = scpi-server
SCPI_PARSER_DIR = $(SCPI_SERVER_DIR)/scpi-parser

.PHONY: scpi

$(SCPI_PARSER_TAR): | $(DL)
	wget $(SCPI_PARSER_URL) -O $@

$(SCPI_PARSER_DIR): $(SCPI_PARSER_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@
#	patch -d $@ -p1 < patches/scpi-parser-$(SCPI_PARSER_TAG).patch

scpi: api $(INSTALL_DIR) $(SCPI_PARSER_DIR)
	$(MAKE) -C $(SCPI_SERVER_DIR)
	$(MAKE) -C $(SCPI_SERVER_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

################################################################################
# Red Pitaya tools
################################################################################

LCR_DIR         = Test/lcr
BODE_DIR        = Test/bode
MONITOR_DIR     = Test/monitor
GENERATE_DIR    = Test/generate
ACQUIRE_DIR     = Test/acquire
CALIB_DIR       = Test/calib
CALIBRATE_DIR   = Test/calibrate
COMM_DIR        = Examples/Communication/C
XADC_DIR        = Test/xadc

.PHONY: examples rp_communication
.PHONY: lcr bode monitor generate acquire calib calibrate

examples: lcr bode monitor generate acquire calib
# calibrate

lcr:
	$(MAKE) -C $(LCR_DIR)
	$(MAKE) -C $(LCR_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

bode:
	$(MAKE) -C $(BODE_DIR)
	$(MAKE) -C $(BODE_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

monitor:
	$(MAKE) -C $(MONITOR_DIR)
	$(MAKE) -C $(MONITOR_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

generate:
	$(MAKE) -C $(GENERATE_DIR)
	$(MAKE) -C $(GENERATE_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

acquire:
	$(MAKE) -C $(ACQUIRE_DIR)
	$(MAKE) -C $(ACQUIRE_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

calib:
	$(MAKE) -C $(CALIB_DIR)
	$(MAKE) -C $(CALIB_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

calibrate: api
	$(MAKE) -C $(CALIBRATE_DIR)
	$(MAKE) -C $(CALIBRATE_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

rp_communication:
	make -C $(COMM_DIR)

#xadc: $(LINUX_DIR)
#	$(MAKE) -C $(XADC_DIR)
#	$(MAKE) -C $(XADC_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

################################################################################
# Red Pitaya OS tools
################################################################################

$(DISCOVERY):
	cp $(OS_TOOLS_DIR)/discovery.sh $@

$(HEARTBEAT):
	cp $(OS_TOOLS_DIR)/heartbeat.sh $@

################################################################################
# Red Pitaya ecosystem and free applications
################################################################################

APPS_FREE_DIR    = apps-free/

ecosystem:
	$(MAKE) -C $(ECOSYSTEM_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

.PHONY: apps-free

apps-free: lcr bode
	$(MAKE) -C $(APPS_FREE_DIR) all
	$(MAKE) -C $(APPS_FREE_DIR) install 

################################################################################
# Red Pitaya PRO applications
################################################################################

ifdef ENABLE_LICENSING

APP_SCOPEGENPRO_DIR = Applications/scopegenpro
APP_SPECTRUMPRO_DIR = Applications/spectrumpro

.PHONY: apps_pro scopegenpro spectrumpro

apps_pro: scopegenpro spectrumpro

scopegenpro: api $(NGINX)
	$(MAKE) -C $(APP_SCOPEGENPRO_DIR)
	$(MAKE) -C $(APP_SCOPEGENPRO_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

spectrumpro: api $(NGINX)
	$(MAKE) -C $(APP_SPECTRUMPRO_DIR)
	$(MAKE) -C $(APP_SPECTRUMPRO_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

else

apps_pro:

endif

################################################################################
# Red Pitaya SDK
################################################################################

sdk:
	$(MAKE) -C $(SDK_DIR) install INSTALL_DIR=$(abspath .)

################################################################################
#
################################################################################

clean:
	-make -C $(LINUX_DIR) clean
	make -C $(FPGA_DIR) clean
	-make -C $(UBOOT_DIR) clean
	make -C shared clean
	# todo, remove downloaded libraries and symlinks
	rm -rf Bazaar/tools/cryptopp
	make -C $(NGINX_DIR) clean
	make -C $(MONITOR_DIR) clean
	make -C $(GENERATE_DIR) clean
	make -C $(ACQUIRE_DIR) clean
	make -C $(CALIB_DIR) clean
	-make -C $(SCPI_SERVER_DIR) clean
	make -C $(LIBRP_DIR)    clean
	make -C $(LIBRPAPP_DIR) clean
	make -C $(SDK_DIR) clean
	make -C $(COMM_DIR) clean
	make -C $(APPS_FREE_DIR) clean
	$(RM) $(INSTALL_DIR) -rf
	$(RM) $(TARGET) -rf
	$(RM) $(NAME)*.zip
