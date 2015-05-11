#
# $Id: $
#
# Red Pitaya library rplib and test app Makefile.
#

APP=$(notdir $(CURDIR:%/=%))
FPGA=fpga.bit

# Versioning system
# BUILD_NUMBER ?= 0
# REVISION ?= devbuild
# VER:=$(shell cat info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')

RPLIB_DIR=rpbase
RPAPPLIB_DIR=rpApplications
RPLIB=$(RPLIB_DIR)/src/librpapp.so
RPAPPLIB=$(RPAPPLIB_DIR)/src/librp.so
ARTIFACTS=$(RPLIB) $(RPAPPLIB)

#CFLAGS += -DVERSION=$(VER)-$(BUILD_NUMBER) -DREVISION=$(REVISION)
#export CFLAGS

all: $(RPLIB) $(RPAPPLIB)

$(RPLIB):
	$(MAKE) -C $(RPLIB_DIR)

$(RPAPPLIB):
	$(MAKE) -C $(RPAPPLIB_DIR)

docapi:
	$(MAKE) -C $(RPLIB_DIR) docapi
	$(MAKE) -C $(RPAPPLIB_DIR) docapi

clean:
	$(MAKE) -C $(RPLIB_DIR) clean
	$(MAKE) -C $(RPAPPLIB_DIR) clean

install:
	$(MAKE) -C $(RPLIB_DIR) install
	$(MAKE) -C $(RPAPPLIB_DIR) install

