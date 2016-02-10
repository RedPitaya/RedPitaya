#
# $Id: $
#
# Red Pitaya library rpApplib and test app Makefile.
#

# Versioning system
# BUILD_NUMBER ?= 0
# REVISION ?= devbuild
# VER:=$(shell cat info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')

LIRPAPP=../lib/librpapp.so
LIBRPAPP_LCR=../lib/librpapp_lcr.so

all: $(LIRPAPP) $(LIBRPAPP_LCR)

$(LIRPAPP):
	$(MAKE) -C src

$(LIBRPAPP_LCR):
	$(MAKE) -C lcr_meter/src

# TODO, there is no documentation directory here
#docapi:
#	$(MAKE) -C doc

clean:
	$(MAKE) -C src clean
#	$(MAKE) -C doc clean

install:
	$(MAKE) -C src install INSTALL_DIR=$(abspath $(INSTALL_DIR))
