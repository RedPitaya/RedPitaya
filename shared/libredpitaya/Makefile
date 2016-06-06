##
# $Id: Makefile 1099 2014-02-12 22:36:35Z ales.bardorfer $
#
# (c) Red Pitaya  http://www.redpitaya.com
#
# Red Pitaya library project file. To build library, run:
# 'make all'
#
# This project file is written for GNU/Make software. For more details please 
# visit: http://www.gnu.org/software/make/manual/make.html
# GNU Compiler Collection (GCC) tools are used for the compilation and linkage. 
# For the details about the usage and building please visit:
# http://gcc.gnu.org/onlinedocs/gcc/
#

# List of compiled object files (not yet linked to executable)
OBJS = system.o http.o
# List of raw source files (all object files, renamed from .o to .c)
SRCS = $(subst .o,.c, $(OBJS)))

# Executable name
TARGET=libredpitaya.so

# GCC compiling & linking flags
CFLAGS=-shared -g -std=gnu99 -Wall -Werror -I../include -lcurl

# Main GCC executable (used for compiling and linking)
CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
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
all: $(TARGET)

# Target with compilation rules to compile object from source files.
# It applies to all files ending with .o. During partial building only new object
# files are created for the source files (.c) which have newer timestamp then 
# objects (.o) files.
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

# Makefile target with rules how to link executable for each target from $(TARGET)
# list.
$(TARGET): $(OBJS)
	$(AR) cr $@ $^


# Clean target - when called it cleans all object files and executables.
clean:
	rm -f $(TARGET) *.o *~

# Install target - creates 'bin/' sub-directory in $(INSTALL_DIR) and copies all
# executables to that location.
install:
	mkdir -p $(INSTALL_DIR)/lib
	cp $(TARGET) $(INSTALL_DIR)/lib
