#
# $Id: Makefile 2015-09-24 18:23:00Z Ulrich Habel (DF4IAH) $
#
# Red Pitaya specific application Makefile for RadioBox.
#

APP=$(notdir $(CURDIR:%/=%))
SHARED_OBJS = 

FPGACFG=fpga.conf
PDF=doc/RedPitaya_RadioBox_Development_Manual.pdf

# Versioning system
BUILD_NUMBER ?= 0
REVISION ?= devbuild
VER:=$(shell grep version info/info.json | sed -E 's/^[^:]*:[^"]*"([^-]*)-.*/\1/')

INSTALL_DIR ?= ../../build

CONTROLLERHF = controllerhf.so
CONTROLLER   = controller.so

CFLAGS += -DVERSION=\"$(VER)-$(BUILD_NUMBER)\" -DREVISION=\"$(REVISION)\"
export CFLAGS


#all:  $(CONTROLLERHF) $(CONTROLLER) doc

all:  $(CONTROLLERHF) doc

#libs:  $(CONTROLLERHF) $(CONTROLLER)

libs:  $(CONTROLLERHF)

$(CONTROLLERHF):
	$(MAKE) -C src clean
	$(MAKE) -C src all OUT_NAME=$(CONTROLLERHF)

$(CONTROLLER):
	$(MAKE) -C src clean
	$(MAKE) -C src all OUT_NAME=$(CONTROLLER) CROSS_COMPILE=arm-linux-gnueabi-

#shared_ctrl: $(CONTROLLERHF) $(CONTROLLER)

shared_ctrl: $(CONTROLLERHF)
	#for obj in $(SHARED_OBJS); do \
		cp $(CONTROLLERHF) $(CONTROLLER) ../$$obj; \
	done
	for obj in $(SHARED_OBJS); do \
		cp $(CONTROLLERHF) ../$$obj; \
	done


doc: $(PDF)

$(PDF):
	$(MAKE) -C doc all


#zip: $(CONTROLLERHF) $(CONTROLLER) $(PDF)

zip: $(CONTROLLERHF) $(PDF)
	mkdir -p target/$(APP)/bin
	mkdir -p target/$(APP)/doc
	#cp -r $(CONTROLLERHF) $(CONTROLLER) $(FPGACFG) bin info index.html target/$(APP)
	cp -r $(CONTROLLERHF) $(FPGA) $(FPGACFG) bin css fonts img info index.html js target/$(APP)
	cp -r doc/*.pdf doc/html target/$(APP)/doc
	sed -i target/$(APP)/info/info.json -e 's/REVISION/$(REVISION)/'
	sed -i target/$(APP)/info/info.json -e 's/BUILD_NUMBER/$(BUILD_NUMBER)/'
	#find target -type f -exec dos2unix {} \;
	cd target; mkdir -p $(INSTALL_DIR); zip -r $(INSTALL_DIR)/$(APP)-$(VER)-$(BUILD_NUMBER)-$(REVISION).zip *
	$(RM) -r target


clean:
	-$(MAKE) -C src clean
	-$(MAKE) -C doc clean
	-$(RM) -r target
	-$(RM) $(CONTROLLERHF) $(CONTROLLER)
	-$(RM) *.zip
