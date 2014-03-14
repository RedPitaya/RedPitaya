#
# $Id: Makefile 1235 2014-02-21 16:44:10Z ales.bardorfer $
#
# Red Pitaya specific application Makefile.
#

APP=$(notdir $(CURDIR:%/=%))
FPGA=fpga.bit

# Versioning system
BUILD_NUMBER ?= 0
REVISION ?= devbuild
VER:=$(shell cat info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')

INSTALL_DIR ?= ../

CONTROLLER=controller.so
ARTIFACTS=$(CONTROLLER)

CFLAGS += -DVERSION=$(VER)-$(BUILD_NUMBER) -DREVISION=$(REVISION)
export CFLAGS

SHARED_SRC=../scope/src

all: $(CONTROLLER)

$(CONTROLLER): src
	$(MAKE) -C src

src:
	cp -r $(SHARED_SRC) src
	$(MAKE) -C src clean

zip: $(CONTROLLER)
	-$(RM) target -rf
	mkdir -p target/$(APP)
	cp -r $(CONTROLLER) $(FPGA) info index.html Makefile target/$(APP)
	$(MAKE) -C src clean
	cp -r src target/$(APP)
	rm `find target/ -iname .svn` -rf
	sed -i target/$(APP)/info/info.json -e 's/REVISION/$(REVISION)/'
	sed -i target/$(APP)/info/info.json -e 's/BUILD_NUMBER/$(BUILD_NUMBER)/'
	cd target; zip -r $(INSTALL_DIR)/$(APP)-$(VER)-$(BUILD_NUMBER)-$(REVISION).zip *
	$(RM) target -rf

clean:
	-$(MAKE) -C src clean
	-$(RM) target -rf
	-$(RM) *.zip
