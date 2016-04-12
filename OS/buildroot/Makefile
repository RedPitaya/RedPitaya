#
# $Id: Makefile 1283 2014-03-01 12:01:39Z ales.bardorfer $
#
# Red Pitaya Ramdisk (root filesystem) Makefile
#

# Where to get buildroot & which version
B_SERVER=http://buildroot.uclibc.org/downloads
B_VERSION=2014.02
B_DIR=buildroot-$(B_VERSION)
B_ARCHIVE=$(B_DIR).tar.gz
B_DOWNLOAD=$(B_SERVER)/$(B_ARCHIVE)
B_TAR=$(DL)/$(B_ARCHIVE)
UIMAGE=$(B_DIR)/output/images/rootfs.cpio.uboot

INSTALL_DIR ?= .
VERSION ?= 0.00-0000
REVISION ?= devbuild

all: $(UIMAGE)

ifeq ($(CROSS_COMPILE),arm-xilinx-linux-gnueabi-)
$(B_DIR)/.config: config.armel
	cp $< $@
else
TOOLCHAIN_PATH=/opt/linaro/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf
$(B_DIR)/.config: config.armhf
	cp $< $@
endif

$(UIMAGE): $(B_DIR) overlay $(B_DIR)/.config
	$(MAKE) -C $(B_DIR) USER_HOOKS_EXTRA_ENV='VERSION=$(VERSION) REVISION=$(REVISION)'

$(B_TAR): | $(DL)
	curl -L $(B_DOWNLOAD) -o $@

$(B_DIR): $(B_TAR)
	tar -xzf $< --strip-components=1 --directory=$@

install: $(UIMAGE)
	mkdir -p $(INSTALL_DIR)
	cp $(UIMAGE) $(INSTALL_DIR)/uramdisk.image.gz
	$(MAKE) -C hostapd install INSTALL_DIR=$(INSTALL_DIR)/sbin

clean:
	-$(MAKE) -C $(B_DIR) clean
	rm *~ -f

mrproper:
	-rm -rf $(B_DIR) $(B_ARCHIVE)
	-rm *~ -f
