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

# Installation directory. It is changed when using the main Makefile++++++
INSTALL_DIR ?= .


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

install:
	mkdir -p $(INSTALL_DIR)/bin
	cp $(SCPISRV) $(INSTALL_DIR)/bin
	mkdir -p $(INSTALL_DIR)/src/utils/$(TARGET)
	-rm -f $(TARGET) *.o
	cp -r * $(INSTALL_DIR)/src/utils/$(TARGET)/
	-rm `find $(INSTALL_DIR)/src/tools/$(TARGET)/ -iname .svn` -rf

clean:
	$(MAKE) -C src clean
	$(MAKE) -C 3rdparty/libs/scpi-parser clean
	$(MAKE) -C ../api-mockup/rpbase/src clean

