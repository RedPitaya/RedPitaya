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

RPLIB=rplib.so
TEST=testApp
ARTIFACTS=$(RPLIB) $(TEST)

#CFLAGS += -DVERSION=$(VER)-$(BUILD_NUMBER) -DREVISION=$(REVISION)
#export CFLAGS

all: $(RPLIB) $(TEST)

$(RPLIB):
	$(MAKE) -C src

$(TEST):
	$(MAKE) -C test

docapi:
	$(MAKE) -C doc

clean:
	$(MAKE) -C src clean
	$(MAKE) -C test clean
	$(MAKE) -C doc clean

