#
# $Id: $
#
# Red Pitaya library librp and test app Makefile.
#

# Versioning system
# BUILD_NUMBER ?= 0
# REVISION ?= devbuild
# VER:=$(shell cat info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')

LIBRP2=../lib/librp2.so

all: $(LIBRP2)

$(LIBRP2):
	$(MAKE) -C src

docapi:
	$(MAKE) -C doc

clean:
	$(MAKE) -C src clean
	$(MAKE) -C doc clean

install:
	$(MAKE) -C src install INSTALL_DIR=$(abspath $(INSTALL_DIR))
