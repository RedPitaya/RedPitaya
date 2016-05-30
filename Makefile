################################################################################
# versioning system
################################################################################

BUILD_NUMBER ?= 0
REVISION ?= devbuild
VER := $(shell cat Applications/ecosystem/info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')
GIT_BRANCH_LOCAL = $(shell echo $(GIT_BRANCH) | sed -e 's/.*\///')
VERSION = $(VER)-$(BUILD_NUMBER)-$(REVISION)
export BUILD_NUMBER
export REVISION
export VERSION

################################################################################
#
################################################################################

INSTALL_DIR=build
# check if download cache directory is available
DL ?= dl

ZIPFILE=ecosystem-$(VERSION).zip

################################################################################
# tarball
################################################################################

all: zip

$(INSTALL_DIR):
	mkdir -p $@

zip: $(ZIPFILE)

$(ZIPFILE): x86 arm
	# copy FPGA bitstream images and decompress them
	mkdir -p $(INSTALL_DIR)/fpga
	cp fpga/archive/*.xz   $(INSTALL_DIR)/fpga
	cd $(INSTALL_DIR)/fpga; xz -df *.xz
	# TODO: check this step
	cp -r OS/filesystem/*  $(INSTALL_DIR)
	@echo "$$GREET_MSG" >  $(INSTALL_DIR)/version.txt
	# copy configuration file for WiFi access point
	cp OS/debian/overlay/etc/hostapd/hostapd.conf $(INSTALL_DIR)/hostapd.conf
	# build zip file
	cd $(INSTALL_DIR); zip -r ../$(ZIPFILE) *

################################################################################
# X86 build (Vivado FPGA synthesis, FSBL, U-Boot, Linux kernel)
################################################################################

.PHONY: x86

x86:
	make -f Makefile.x86

################################################################################
# ARM userspace
################################################################################

.PHONY: arm

arm:
#	chroot $(ROOT_DIR) make -C git -f Makefile.arm DL=/dl

################################################################################
# local (on RP board) install process
# TODO: generalized install should be provided instead
################################################################################

localinstall:
	systemctl stop redpitaya_nginx
	rw
	/bin/cp build/sbin/* /opt/redpitaya/sbin/
	/bin/cp build/lib/* /opt/redpitaya/lib/
	/bin/cp -r build/www/apps/* /opt/redpitaya/www/apps/
	ro
	systemctl start redpitaya_nginx

ROOT_IMG  = $(DL)/red_pitaya_OS_23-Mar-2016.img
ROOT_DIR  = sysroot
#ROOT_DEV := $(shell `losetup -f`)

# should be run as root (sudo)
root_mount:
	# setup loop device
	losetup -v $(ROOT_DEV) -o 127926272 $(ROOT_IMG)
#	losetup -P -f $(DL)/$(ROOT_IMG)
	mkdir -p                    $(ROOT_DIR)
	# mount image
#	mount -o ro $(ROOT_DEV)     $(ROOT_DIR)
	mount       $(ROOT_DEV)     $(ROOT_DIR)
	# mount runtime directories
	mount --bind /proc          $(ROOT_DIR)/proc
	mount --bind /tmp           $(ROOT_DIR)/tmp
	mount --bind /sys           $(ROOT_DIR)/sys
	mount --bind /dev           $(ROOT_DIR)/dev
	mount --bind /dev/pts       $(ROOT_DIR)/dev/pts
	# mount git project
	mkdir -p                    $(ROOT_DIR)/git
	mount --bind `pwd`          $(ROOT_DIR)/git
	# mount download directory
	mkdir -p                    $(ROOT_DIR)/dl
	mount --bind $(DL)          $(ROOT_DIR)/dl
	# set prompt
#	echo ubuntu-arm           > $(ROOT_DIR)/etc/debian_chroot
	# copy networking configuration
	cp /etc/resolv.conf         $(ROOT_DIR)/etc/
	# copy emulator
	cp /usr/bin/qemu-arm-static $(ROOT_DIR)/usr/bin/

root_umount:
	# remove emulator
	rm                          $(ROOT_DIR)/usr/bin/qemu-arm-static
	# umount image
	umount -l                   $(ROOT_DIR)
	# detach loop device
	losetup -d  $(ROOT_DEV)

################################################################################
#
################################################################################

clean:
	$(RM) $(INSTALL_DIR) -rf
	$(RM) $(ZIPFILE)
