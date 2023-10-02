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

CONTROLLER=controllerhf.so
TOOL=lcr
ZIP=../../$(APP)-$(VER)-$(BUILD_NUMBER)-$(REVISION).zip
ARTIFACTS=$(CONTROLLER)

CFLAGS += -DVERSION=$(VER)-$(BUILD_NUMBER) -DREVISION=$(REVISION)
export CFLAGS

all: $(CONTROLLER) tool

$(CONTROLLER):
	$(MAKE) -C src 

$(ZIP): $(CONTROLLER)
	-$(RM) target -rf
	mkdir -p target/$(APP)
	cp -r $(CONTROLLER) nginx.conf fpga.conf fpga.sh css img *.html info js target/$(APP)
	sed -i target/$(APP)/info/info.json -e 's/REVISION/$(REVISION)/'
	sed -i target/$(APP)/info/info.json -e 's/BUILD_NUMBER/$(BUILD_NUMBER)/'
	cd target; zip -r ../$(ZIP) *
	$(RM) target -rf

tool:
	$(MAKE) -C src  lcr

install: $(ZIP)
	unzip -o $(ZIP) -d $(INSTALL_DIR)/www/apps
	mkdir -p $(INSTALL_DIR)/bin
	cp $(TOOL) $(INSTALL_DIR)/bin

clean:
	$(MAKE) -C src clean
	-$(RM) target -rf
	-$(RM) $(ZIP)
