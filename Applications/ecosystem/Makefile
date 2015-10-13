#
# $Id: Makefile 1232 2014-02-21 15:38:14Z ales.bardorfer $
#
# Red Pitaya specific ecosystem Makefile.
#

APP=$(notdir $(CURDIR:%/=%))

# Versioning system
BUILD_NUMBER ?= 0
REVISION ?= devbuild
VER:=$(shell cat info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')

INSTALL_DIR ?= ../

all: zip

install:
	mkdir -p $(INSTALL_DIR)/www/apps/
	cp -r ../assets images info ui *.html style.css $(INSTALL_DIR)/www/apps/
	sed -i $(INSTALL_DIR)/www/apps/info/info.json -e 's/REVISION/$(REVISION)/'
	sed -i $(INSTALL_DIR)/www/apps/info/info.json -e 's/BUILD_NUMBER/$(BUILD_NUMBER)/'

zip:
	mkdir -p target/www/apps/
	cp -r ../assets images info ui *.html style.css target/www/apps/
	sed -i target/www/apps/info/info.json -e 's/REVISION/$(REVISION)/'
	sed -i target/www/apps/info/info.json -e 's/BUILD_NUMBER/$(BUILD_NUMBER)/'
	cd target; zip -r $(INSTALL_DIR)/redpitaya-$(APP)-$(VER)-$(BUILD_NUMBER)-$(REVISION).zip *
	rm target -rf

clean:
	-$(RM) target *.zip -rf
	-$(RM) `find . -name \*~` -rf
