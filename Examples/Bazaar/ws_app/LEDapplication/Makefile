#
# $Id: Makefile 1235 2016-06-6 16:42:10 JamesW $
#
# Red Pitaya LED application Makefile.
#

APP=$(notdir $(CURDIR:%/=%))

# Versioning system
BUILD_NUMBER ?= 0
REVISION ?= devbuild
VER:=$(shell cat info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')


CONTROLLERHF = controllerhf.so

ARTIFACTS=$(CONTROLLER)

CFLAGS += -DVERSION=$(VER)-$(BUILD_NUMBER) -DREVISION=$(REVISION)

export CFLAGS

all: $(CONTROLLERHF)

$(CONTROLLERHF):
	$(MAKE) -C rp_sdk
	$(MAKE) -C src

install: $(CONTROLLERHF)
	-$(RM) target -rf
	mkdir -p target/$(APP)
	cp -r $(CONTROLLERHF) fpga.conf info index.html target/$(APP)
	sed -i target/$(APP)/info/info.json -e 's/REVISION/$(REVISION)/'
	sed -i target/$(APP)/info/info.json -e 's/BUILD_NUMBER/$(BUILD_NUMBER)/'
	cp -r -u target/$(APP)  /opt/redpitaya/www/apps/$(APP)

clean:
	$(MAKE) -C src clean
	$(MAKE) -C rp_sdk clean
	-$(RM) target -rf
	-$(RM) *.so
