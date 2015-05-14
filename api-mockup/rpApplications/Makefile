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

all: $(LIRPAPP)

$(LIRPAPP):
	$(MAKE) -C src

# TODO, there is no documentation directory here
#docapi:
#	$(MAKE) -C doc

clean:
	$(MAKE) -C src clean
#	$(MAKE) -C doc clean

install:
	$(MAKE) -C src install
