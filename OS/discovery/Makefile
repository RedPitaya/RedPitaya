##
# $Id: Makefile 1252 2014-02-23 21:07:38Z ales.bardorfer $
#
# (c) Red Pitaya  http://www.redpitaya.com
#
# IP discovery project file. To build executable, run:
# 'make all'
#
# This project file is written for GNU/Make software. For more details please 
# visit: http://www.gnu.org/software/make/manual/make.html
# GNU Compiler Collection (GCC) tools are used for the compilation and linkage. 
# For the details about the usage and building please visit:
# http://gcc.gnu.org/onlinedocs/gcc/
#

# Versioning system
VERSION ?= 0.00-0000
REVISION ?= devbuild

# List of compiled object files (not yet linked to executable)
OBJS = discovery.o cJSON.o
# List of raw source files (all object files, renamed from .o to .c)
SRCS = $(subst .o,.c, $(OBJS)))

# Executable name
TARGET=discovery

# GCC compiling & linking flags
CFLAGS=-g -std=gnu99 -Wall -Werror -I../../shared/include
CFLAGS += -DVERSION=$(VERSION) -DREVISION=$(REVISION)
# Additional libraries which needs to be dynamically linked to the executable
# -lm - System math library (used by cos(), sin(), sqrt(), ... functions)
BUILDROOT_PATH=../../OS/buildroot/buildroot-2014.02/output/build
SHARED_PATH=../../shared
LIBS=-lm
LIBS+= -lredpitaya -L$(SHARED_PATH)/libredpitaya
LIBS+= -lcurl -L$(BUILDROOT_PATH)/libcurl-7.35.0/lib/.libs
LIBS+= -lssl -lcrypto -L$(BUILDROOT_PATH)/openssl-1.0.1f
LIBS+= -lz -lcrypto -L$(BUILDROOT_PATH)/zlib-1.2.8

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
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

# Clean target - when called it cleans all object files and executables.
clean:
	rm -f $(TARGET) *.o *~

# Install target - creates 'bin/' sub-directory in $(INSTALL_DIR) and copies all
# executables to that location.
install:
	mkdir -p $(INSTALL_DIR)/sbin
	cp $(TARGET) $(INSTALL_DIR)/sbin
