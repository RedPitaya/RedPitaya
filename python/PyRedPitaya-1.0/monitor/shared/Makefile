##
# $Id: Makefile 1063 2014-02-11 16:15:56Z ales.bardorfer $
#
# (c) Red Pitaya  http://www.redpitaya.com
#
# Red Pitaya shared libraries project file. To build, run:
# 'make all'
#
# This project file is written for GNU/Make software. For more details please 
# visit: http://www.gnu.org/software/make/manual/make.html
# GNU Compiler Collection (GCC) tools are used for the compilation and linkage. 
# For the details about the usage and building please visit:
# http://gcc.gnu.org/onlinedocs/gcc/
#


CURL_DIR=curl
CURL_INSTALL_DIR=$(CURL_DIR)/target
LIBCURL=$(CURL_INSTALL_DIR)/lib/libcurl.a
LIBREDPITAYA=libredpitaya/libredpitaya.a

# Main GCC executable (used for compiling and linking)
CC=$(CROSS_COMPILE)gcc
# Installation directory
INSTALL_DIR ?= .

# Makefile is composed of so called 'targets'. They give basic structure what 
# needs to be execued during various stages of the building/removing/installing
# of software package.
# Simple Makefile targets have the following structure:
# <name>: <dependencies>
#	<command1>
#       <command2>
#       ...
# The target <name> is completed in the following order:
#   - list od <dependencies> finished
#   - all <commands> in the body of targets are executed succsesfully

# Main Makefile target 'all' - it iterates over all targets listed in $(TARGET)
# variable.
all: $(LIBREDPITAYA) $(LIBCURL)

$(LIBCURL):
	[ -f $(CURL_DIR)/config.status ] || { cd $(CURL_DIR); ./configure --host=arm-xilinx-linux-gnueabi --build=i686-linux --enable-smtp --prefix=$(abspath $(CURL_INSTALL_DIR)); }
	$(MAKE) -C $(CURL_DIR)
	$(MAKE) -C $(CURL_DIR) install

$(LIBREDPITAYA):
	$(MAKE) -C libredpitaya

# Clean target - when called it cleans all object files and executables.
clean:
	rm -rf $(CURL_INSTALL_DIR)
	$(MAKE) -C libredpitaya clean
	rm -f *~

distclean: clean
	$(MAKE) -C $(CURL_DIR) clean

# Install target - creates 'bin/' sub-directory in $(INSTALL_DIR) and copies all
# executables to that location.
install:
	mkdir -p $(INSTALL_DIR)/lib
	cp $(LIBREDPITAYA) $(INSTALL_DIR)/lib
	cp -r $(CURL_INSTALL_DIR)/lib/ $(INSTALL_DIR)
