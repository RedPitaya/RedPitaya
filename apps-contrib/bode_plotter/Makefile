#
# $Id: Makefile 1235 2014-02-21 16:44:10Z ales.bardorfer, luka.golinar $
#
# Contributed apps application specific apps
#

APP=$(notdir $(CURDIR:%/=%))

# Versioning system
BUILD_NUMBER ?= 0
REVISION ?= devbuild
VER:=$(shell cat info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')

CONTROLLER   = controller.so
CONTROLLERHF = controllerhf.so

all: $(CONTROLLER) $(CONTROLLERHF)

$(CONTROLLER):
	$(MAKE) -C src clean
	$(MAKE) -C src all OUT_NAME=$(CONTROLLER) CROSS_COMPILE=arm-xilinx-linux-gnueabi-

$(CONTROLLERHF):
	$(MAKE) -C src clean
	$(MAKE) -C src all OUT_NAME=$(CONTROLLERHF)

zip: $(CONTROLLER) $(CONTROLLERHF)
	-$(RM) target -rf
	mkdir -p target/$(APP)
	cp -r $(CONTROLLER) $(CONTROLLERHF) fpga.conf info index.html Makefile target/$(APP)
	$(MAKE) -C src clean
	cp -r src target/$(APP)
	cd target; zip -r $(INSTALL_DIR)/$(APP)-$(VER)-$(BUILD_NUMBER)-$(REVISION).zip *
	-$(RM) target -rf

clean:
	$(MAKE) -C src clean
	-$(RM) target -rf
	-$(RM) *.so
