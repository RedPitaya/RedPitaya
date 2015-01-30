#
# $Id$
#
# RedPitaya SDK Makefile
#
# Produces:
#   1. SDK zip
#

#Using shell commands

#Versioning system
BUILD_NUMBER ?= 0
REVISION ?= devbuild
#VER:=$(shell cat info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')

ZIP_NAME = SDK
RP_INCLUDE = ./include
GREET_MSG = "\n---------- INSTALLING REDPITAYA SDK ZIP ----------\n"

all: install

install:
	@echo "\n---------- INSTALLING REDPITAYA SDK ZIP ----------\n"
	chmod +x ./install.sh
	zip -r $(ZIP_NAME)-$(BUILD_NUMBER)-$(REVISION).zip * -x "./Makefile"

clean:
	rm -rf *.zip
	
	


