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

SCPIPARSER=scpi-parser/libscpi/dist/libscpi.so
SCPISRV=scpi-server
ARTIFACTS= $(SCPIPARSER) $(SCPISRV)


all: $(SCPIPARSER) $(SCPISRV)

$(SCPISRV):
	$(MAKE) -C src

$(SCPIPARSER):
	$(MAKE) -C scpi-parser CC=$(CROSS_COMPILE)gcc USER_FULL_ERROR_LIST=1

install:
	mkdir -p $(INSTALL_DIR)/bin
	mkdir -p $(INSTALL_DIR)/lib
	cp $(SCPISRV)    $(INSTALL_DIR)/bin
	cp $(SCPIPARSER) $(INSTALL_DIR)/lib

clean:
	$(MAKE) -C src clean
	$(MAKE) -C scpi-parser clean
