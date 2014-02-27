#
# $Id$
#
# Linux kernel Makefile
#

LINUX_GIT := /var/git/linux-xlnx.git
LINUX_TAG := xilinx-v14.6.02
XLINUX_DIR := linux-xlnx
PATCH := patches
NUM_CPU := $(shell grep -c ^processor /proc/cpuinfo)
INSTALL_DIR ?= .

.PHONY : all install clean distclean

all: $(XLINUX_DIR)
	make -j $$(($(NUM_CPU)+1)) -C $(XLINUX_DIR) ARCH=arm \
	CROSS_COMPILE=arm-xilinx-linux-gnueabi- UIMAGE_LOADADDR=0x8000 uImage

$(XLINUX_DIR):
	mkdir $(XLINUX_DIR)
	cd $(XLINUX_DIR); git archive --remote $(LINUX_GIT) $(LINUX_TAG) | tar x
	cat $(PATCH)/*.patch | patch -p1 -d $(XLINUX_DIR)
	cp ./config/rp_defconfig $(XLINUX_DIR)/arch/arm/configs/
	make -C $(XLINUX_DIR) mrproper
	make -C $(XLINUX_DIR) ARCH=arm rp_defconfig

install:
	cp $(XLINUX_DIR)/arch/arm/boot/uImage $(INSTALL_DIR)

clean:
	-make -C $(XLINUX_DIR) ARCH=arm clean

distclean:
	rm -rf $(XLINUX_DIR)
