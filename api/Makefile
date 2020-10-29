#
# $Id: $
#
# Red Pitaya library librp and test app Makefile.
#

# Versioning system
# BUILD_NUMBER ?= 0
# REVISION ?= devbuild
# VER:=$(shell cat info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')

MODEL ?= Z10

LIBRP=lib/librp.so

all: $(LIBRP)

$(LIBRP):
	$(MAKE) -C src INSTALL_DIR=$(abspath $(INSTALL_DIR)) -j4

clean:
	$(MAKE) -C src clean

install:
	$(MAKE) -C src install INSTALL_DIR=$(abspath $(INSTALL_DIR))
