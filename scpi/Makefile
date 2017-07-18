#
# $Id: $
#
# Red Pitaya library librp and test app Makefile.
#

APP=$(notdir $(CURDIR:%/=%))

# Versioning system
# BUILD_NUMBER ?= 0
# REVISION ?= devbuild
# VER:=$(shell cat info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')

# Installation directory. It is changed when using the main Makefile++++++
INSTALL_DIR ?= .

LIBSCPI=scpi-parser/libscpi/dist/libscpi.so.2.1.0
SCPISRV=scpi-server
ARTIFACTS= $(LIBSCPI) $(SCPISRV)

all: $(LIBSCPI) $(SCPISRV)

$(SCPISRV):
	$(MAKE) -C src

$(LIBSCPI):
	$(MAKE) -C scpi-parser CC=$(CROSS_COMPILE)gcc USER_FULL_ERROR_LIST=1

install:
	mkdir -p $(INSTALL_DIR)/bin
	mkdir -p $(INSTALL_DIR)/lib
	cp $(SCPISRV) $(INSTALL_DIR)/bin
	$(MAKE) -C scpi-parser install PREFIX=$(INSTALL_DIR)
	ln -sf libscpi.so.2.1.0 $(INSTALL_DIR)/lib/libscpi.so

clean:
	$(MAKE) -C src clean
	$(MAKE) -C scpi-parser clean
