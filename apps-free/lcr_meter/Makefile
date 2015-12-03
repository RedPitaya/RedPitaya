#
# Authors: Aleš Bardorfer, Luka Golinar
#
# Red Pitaya specific application Makefile.
#

APP=$(notdir $(CURDIR:%/=%))

# Versioning system
BUILD_NUMBER ?= 0
REVISION ?= devbuild
VER:=$(shell grep version info/info.json | sed -E 's/^[^:]*:[^"]*"([^-]*)-.*/\1/')

INSTALL_DIR ?= ../../build

CONTROLLERHF = controllerhf.so
LCR          = ../../build/bin/lcr

CFLAGS += -DVERSION=$(VER)-$(BUILD_NUMBER) -DREVISION=$(REVISION)
export CFLAGS

all: $(CONTROLLERHF)

$(CONTROLLERHF):
	$(MAKE) -C src

$(LCR):
	$(MAKE) -C ../../ lcr

zip: $(CONTROLLERHF)
	-$(RM) -r target
	mkdir -p target/$(APP)
	cp -r $(CONTROLLERHF) $(LCR) fpga.conf info index.html style.css images target/$(APP)
	sed -i target/$(APP)/info/info.json -e 's/REVISION/$(REVISION)/'
	sed -i target/$(APP)/info/info.json -e 's/BUILD_NUMBER/$(BUILD_NUMBER)/'
	cd target; zip -r $(INSTALL_DIR)/$(APP)-$(VER)-$(BUILD_NUMBER)-$(REVISION).zip *
	$(RM) -r target

clean:
	$(MAKE) -C src clean
	-$(RM) -r target
	-$(RM) *.so
