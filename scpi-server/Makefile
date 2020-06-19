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
MODEL ?= Z10

LIBSCPI=scpi-parser/libscpi/dist/libscpi.a
SCPISRV=scpi-server
ARTIFACTS= $(LIBSCPI) $(SCPISRV)

all: $(LIBSCPI) $(SCPISRV)

$(SCPISRV):
	$(MAKE) -C src MODEL=$(MODEL) INSTALL_DIR=$(abspath $(INSTALL_DIR))

$(LIBSCPI):
	$(MAKE) -C scpi-parser CC=$(CROSS_COMPILE)gcc USER_FULL_ERROR_LIST=1

install:
	mkdir -p $(INSTALL_DIR)/bin
	cp $(SCPISRV) $(INSTALL_DIR)/bin

clean:
	$(MAKE) -C src clean
	$(MAKE) -C scpi-parser clean
