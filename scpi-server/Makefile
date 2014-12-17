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

# INSTALL_DIR ?= ../


SCPIPARSER=3rdparty/libs/scpi-parser/libscpi/dist/libscpi.so
SCPISRV=scpi-server
RPLIB=../api-mockup/rpbase/librp.so
ARTIFACTS= $(RPLIB) $(SCPIPARSER) $(SCPISRV)

#CFLAGS += -DVERSION=$(VER)-$(BUILD_NUMBER) -DREVISION=$(REVISION)
#export CFLAGS

all: $(RPLIB) $(SCPIPARSER) $(SCPISRV)

$(RPLIB):
	$(MAKE) -C ../api-mockup/rpbase/src

$(SCPISRV):
	$(MAKE) -C src

$(SCPIPARSER):
	$(MAKE) -C 3rdparty/libs/scpi-parser

clean:
	$(MAKE) -C src clean
	$(MAKE) -C 3rdparty/libs/scpi-parser clean
	$(MAKE) -C ../api-mockup/rpbase/src clean

