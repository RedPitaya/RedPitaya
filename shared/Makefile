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

LIBREDPITAYA=libredpitaya/libredpitaya.so

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
all: $(LIBREDPITAYA)

$(LIBREDPITAYA):
	$(MAKE) -C libredpitaya

# Clean target - when called it cleans all object files and executables.
clean:
	$(MAKE) -C libredpitaya clean
	rm -f *~

# Install target - creates 'bin/' sub-directory in $(INSTALL_DIR) and copies all
# executables to that location.
install:
	mkdir -p $(INSTALL_DIR)/lib
	cp $(LIBREDPITAYA) $(INSTALL_DIR)/lib
