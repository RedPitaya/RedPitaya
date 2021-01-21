#
# $Id: Makefile 1235 2014-02-21 16:44:10Z ales.bardorfer $
#
# Red Pitaya specific application Makefile.
#

APP=$(notdir $(CURDIR:%/=%))

MODEL ?= Z10

# Versioning system
BUILD_NUMBER ?= 0
REVISION ?= devbuild
VER:=$(shell cat info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')

INSTALL_DIR ?= .

CONTROLLER=controllerhf.so
SERVICE=streaming-server
ZIP=../../$(APP)-$(VER)-$(BUILD_NUMBER)-$(REVISION).zip
ARTIFACTS=$(CONTROLLER)

CFLAGS += -DVERSION=$(VER)-$(BUILD_NUMBER) -DREVISION=$(REVISION)
export CFLAGS

.PHONY: all
all: $(CONTROLLER)

$(CONTROLLER):
	$(MAKE) -C src MODEL=$(MODEL)

$(ZIP): $(CONTROLLER)
	-$(RM) target -rf
	echo "#!/bin/sh" > fpga.sh
ifeq ($(MODEL),Z20_250_12)
	echo '/opt/redpitaya/sbin/mkoverlay.sh stream_app_250' >> fpga.sh
else
	echo '/opt/redpitaya/sbin/mkoverlay.sh stream_app' >> fpga.sh
endif
	mkdir -p target/$(APP)
	cp -r $(CONTROLLER) fpga.conf nginx.conf css img *.html fpga.sh info js target/$(APP)
	mv $(SERVICE) $(INSTALL_DIR)/bin
	sed -i target/$(APP)/info/info.json -e 's/REVISION/$(REVISION)/'
	sed -i target/$(APP)/info/info.json -e 's/BUILD_NUMBER/$(BUILD_NUMBER)/'
	cd target; zip -r ../$(ZIP) *
	$(RM) target -rf

install: $(ZIP)
	unzip -o $(ZIP) -d $(INSTALL_DIR)/www/apps

clean:
	$(MAKE) -C src clean
	-$(RM) target -rf
	-$(RM) $(SERVICE)
	-$(RM) $(ZIP)
