#
# $Id: Makefile 1235 2014-02-21 16:44:10Z ales.bardorfer $
#
# Red Pitaya specific application Makefile.
#

APP=$(notdir $(CURDIR:%/=%))
SHARED_OBJS = scope+gen scope+pid

# Versioning system
BUILD_NUMBER ?= 0
REVISION ?= devbuild
VER:=$(shell cat info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')

INSTALL_DIR ?= .

CONTROLLERHF = controllerhf.so
ZIP = ../../$(APP)-$(VER)-$(BUILD_NUMBER)-$(REVISION).zip

CFLAGS += -DVERSION=$(VER)-$(BUILD_NUMBER) -DREVISION=$(REVISION)
export CFLAGS

all: $(CONTROLLERHF) shared_ctrl

$(CONTROLLERHF):
	$(MAKE) -C src

shared_ctrl: $(CONTROLLERHF)
	for obj in $(SHARED_OBJS); do \
		cp $(CONTROLLERHF) ../$$obj; \
	done
	
$(ZIP): $(CONTROLLERHF)
	-$(RM) target -rf
	mkdir -p target/$(APP)
	cp -r $(CONTROLLERHF) fpga.conf info index.html target/$(APP)
	sed -i target/$(APP)/info/info.json -e 's/REVISION/$(REVISION)/'
	sed -i target/$(APP)/info/info.json -e 's/BUILD_NUMBER/$(BUILD_NUMBER)/'
	cd target; zip -r ../$(ZIP) *
	$(RM) target -rf

install: $(ZIP)
	unzip $(ZIP) -d $(INSTALL_DIR)/www/apps

clean:
	$(MAKE) -C src clean
	-$(RM) target -rf
	-$(RM) *.so
