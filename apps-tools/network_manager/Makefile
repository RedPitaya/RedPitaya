#
# $Id: Makefile 1235 2014-02-21 16:44:10Z ales.bardorfer $
#
# Red Pitaya specific application Makefile.
#

APP=$(notdir $(CURDIR:%/=%))

# Versioning system
BUILD_NUMBER ?= 0
REVISION ?= devbuild
VER:=$(shell cat info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')

INSTALL_DIR ?= .
TARGET = $(INSTALL_DIR)/www/apps/$(APP)

install:
	mkdir -p $(TARGET)
	cp -r nginx.conf fpga.conf css img *.html info js iw_scan.awk apmode_def_route.sh $(TARGET)
	sed -i $(TARGET)/info/info.json -e 's/REVISION/$(REVISION)/'
	sed -i $(TARGET)/info/info.json -e 's/BUILD_NUMBER/$(BUILD_NUMBER)/'
