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

CONTROLLER=daemon

CFLAGS += -DVERSION=$(VER)-$(BUILD_NUMBER) -DREVISION=$(REVISION)
export CFLAGS

all: $(CONTROLLER)

$(CONTROLLER):
	$(MAKE) -C src

install: $(CONTROLLER)
	mkdir -p $(TARGET)

	cp -r $(CONTROLLER) nginx.conf fpga.conf css img *.html info js *.sh $(TARGET)

	sed -i $(TARGET)/info/info.json -e 's/REVISION/$(REVISION)/'
	sed -i $(TARGET)/info/info.json -e 's/BUILD_NUMBER/$(BUILD_NUMBER)/'

clean:
	$(MAKE) -C src clean