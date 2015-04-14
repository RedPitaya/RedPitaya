#
# $Id: $
#
# Red Pitaya library librp and test app Makefile.
#

APP=$(notdir $(CURDIR:%/=%))
FPGA=fpga.bit

# Versioning system
# BUILD_NUMBER ?= 0
# REVISION ?= devbuild
# VER:=$(shell cat info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')

LIBRP=librp.so
TEST=testApp
ARTIFACTS=$(LIBRP) $(TEST)

#CFLAGS += -DVERSION=$(VER)-$(BUILD_NUMBER) -DREVISION=$(REVISION)
#export CFLAGS

all: $(LIBRP) $(TEST)

$(LIBRP):
	$(MAKE) -C src

$(TEST):
	$(MAKE) -C test

docapi:
	$(MAKE) -C doc

clean:
	$(MAKE) -C src clean
	$(MAKE) -C test clean
	$(MAKE) -C doc clean

