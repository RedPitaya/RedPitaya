#
# $Id: Makefile 1235 2014-02-21 16:44:10Z ales.bardorfer $
#
# Red Pitaya specific application Makefile.
#

APP=$(notdir $(CURDIR:%/=%))

# Versioning system
BUILD_NUMBER ?= 0
REVISION ?= devbuild
VER:=$(shell grep version info/info.json | sed -E 's/[^:]*:[^"]*"([^-]*)-.*$/\1/')

INSTALL_DIR ?= ../../build

CONTROLLERHF = controllerhf.so
CONTROLLER   = controller.so

CFLAGS += -DVERSION=$(VER)-$(BUILD_NUMBER) -DREVISION=$(REVISION)
export CFLAGS

all: $(CONTROLLERHF) $(CONTROLLER)

$(CONTROLLERHF):
	$(MAKE) -C src clean
	$(MAKE) -C src OUT_NAME=$(CONTROLLERHF)

$(CONTROLLER):
	$(MAKE) -C src clean
	$(MAKE) -C src OUT_NAME=$(CONTROLLER) CROSS_COMPILE=arm-xilinx-linux-gnueabi-

zip: $(CONTROLLERHF) $(CONTROLLER)
	-$(RM) target -rf
	mkdir -p target/$(APP)
	cp -r $(CONTROLLERHF) $(CONTROLLER) fpga.conf info index.html target/$(APP)
	touch target/$(APP)/fpga.bit
	sed -i target/$(APP)/info/info.json -e 's/REVISION/$(REVISION)/'
	sed -i target/$(APP)/info/info.json -e 's/BUILD_NUMBER/$(BUILD_NUMBER)/'
	cd target; zip -r $(INSTALL_DIR)/$(APP)-$(VER)-$(BUILD_NUMBER)-$(REVISION).zip *
	$(RM) target -rf

clean:
	$(MAKE) -C src clean
	-$(RM) *.so
