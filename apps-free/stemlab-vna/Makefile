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
VIVADO = /opt/Xilinx/Vivado/2017.2/bin/vivadoO -nolog -nojournal -mode batch

INSTALL_DIR ?= .

CONTROLLERHF = controllerhf.so
SERVER = vna
BLOCK_DESIGN_FPGA = block_design.bit
ZIP = ../../$(APP)-$(VER)-$(BUILD_NUMBER)-$(REVISION).zip

CFLAGS += -DVERSION=$(VER)-$(BUILD_NUMBER) -DREVISION=$(REVISION)
export CFLAGS

all: $(CONTROLLERHF) $(SERVER) $(BLOCK_DESIGN_FPGA)

$(CONTROLLERHF):
	$(MAKE) -C src

$(SERVER):
	$(MAKE) -C server

$(BLOCK_DESIGN_FPGA):
	$(VIVADO) -source block_design.tcl -tclargs .

$(ZIP): $(CONTROLLERHF) $(SERVER) $(BLOCK_DESIGN_FPGA)
	-$(RM) target -rf
	mkdir -p target/$(APP)
	cp -r $(CONTROLLERHF) $(SERVER) $(BLOCK_DESIGN_FPGA) fpga.conf info index.html target/$(APP)
	sed -i target/$(APP)/info/info.json -e 's/REVISION/$(REVISION)/'
	sed -i target/$(APP)/info/info.json -e 's/BUILD_NUMBER/$(BUILD_NUMBER)/'
	cd target; zip -r ../$(ZIP) *
	$(RM) target -rf

install: $(ZIP)
	unzip $(ZIP) -d $(INSTALL_DIR)/www/apps

clean:
	$(MAKE) -C src clean
	$(MAKE) -C server clean
	-$(RM) target -rf
	-$(RM) *.so
	-$(RM) $(ZIP)
