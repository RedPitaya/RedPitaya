#
# $Id: $
#
# Red Pitaya library rpApplib and test app Makefile.
#

APP=$(notdir $(CURDIR:%/=%))
FPGA=fpga.bit

# Versioning system
# BUILD_NUMBER ?= 0
# REVISION ?= devbuild
# VER:=$(shell cat info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')

RPLIB=rpApplib.so
ARTIFACTS=$(RPLIB) $(TEST)

#CFLAGS += -DVERSION=$(VER)-$(BUILD_NUMBER) -DREVISION=$(REVISION)
#export CFLAGS

all: $(RPLIB)

$(RPLIB):
	$(MAKE) -C src

docapi:
	$(MAKE) -C doc

clean:
	$(MAKE) -C src clean
	$(MAKE) -C doc clean

