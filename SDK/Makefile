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

CC = CROSS_COMPILE=arm-linux-gnueabi-
NAME = SDK
#Here I use the same var for rplib and rp.h. It makes Makefile cleaner, but it gives less modulation when changing either of the two file location.
#We could make another var just for rp.h
RP_LIBRARY_INCLUDE = ../api-mockup/rpbase/src

RP_SCPI_LIB_INCLUDE = ../scpi-server/3rdparty/libs/scpi-parser/libscpi/dist/
GREET_MSG = "\nINSTALLING REDPITAYA SDK ZIP...\n"

all: zip
	
zip:
	@echo $(GREET_MSG)
	mkdir -p include
	cp $(RP_LIBRARY_INCLUDE)/rp.h ./include
	#Make rplib, if we are making SDK without the main makefile
	$(MAKE) -C $(RP_LIBRARY_INCLUDE) $(CC)
	cp $(RP_LIBRARY_INCLUDE)/librp.so ./include
	#Make scpi server, if we are making SDK without the main makefile
	$(MAKE) -C ../scpi-server clean
	$(MAKE) -C ../scpi-server/ $(CC)
	cp $(RP_SCPI_LIB_INCLUDE)/* ./include
	rm ./include/libscpi.a
	chmod +x ./install.sh
	
	mkdir -p ./pitaya_remote_debug_example/blink_diode
	cp ../Examples/C/blink_diode.c ./pitaya_remote_debug_example/blink_diode
	cp ../Examples/C/Makefile ./pitaya_remote_debug_example/blink_diode
	zip -r ../$(NAME)-$(VERSION)-$(BUILD_NUMBER)-$(REVISION).zip * -x "./Makefile" 

clean:
	rm -rf include
	rm -rf *.zip
	
	


