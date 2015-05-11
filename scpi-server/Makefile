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
API_DIR=../api-mockup/

CROSS_COMPILE?=arm-linux-gnueabi-

SCPIPARSER=3rdparty/libs/scpi-parser/libscpi/dist/libscpi.so
SCPISRV=scpi-server
LIBRP=../api-mockup/lib/librp.so
LIBRPAPP=../api-mockup/lib/librpapp.so
ARTIFACTS= $(LIBRP) $(LIBRPAPP) $(SCPIPARSER) $(SCPISRV)

all: $(LIBRP) $(LIBRPAPP) $(SCPIPARSER) $(SCPISRV)

$(LIBRP):
	$(MAKE) -C $(API_DIR)/rpbase

$(LIBRPAPP):
	$(MAKE) -C $(API_DIR)/rpApplications

$(SCPISRV):
	$(MAKE) -C src

$(SCPIPARSER):
	$(MAKE) -C 3rdparty/libs/scpi-parser CROSS_COMPILE=$(CROSS_COMPILE)

install:
	mkdir -p $(INSTALL_DIR)/bin
	mkdir -p $(INSTALL_DIR)/lib
	cp $(SCPISRV)    $(INSTALL_DIR)/bin
	cp $(SCPIPARSER) $(INSTALL_DIR)/lib
	make -C $(API_DIR)/rpbase         INSTALL_DIR=$(INSTALL_DIR)/lib install
	make -C $(API_DIR)/rpApplications INSTALL_DIR=$(INSTALL_DIR)/lib install

clean:
	$(MAKE) -C src clean
	$(MAKE) -C 3rdparty/libs/scpi-parser clean
	$(MAKE) -C $(API_DIR)/rpbase
	$(MAKE) -C $(API_DIR)/rpApplications
