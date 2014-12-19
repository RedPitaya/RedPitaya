##
# $Id: $
#
# (c) Red Pitaya  http://www.redpitaya.com
#
# Library rplib project file. To build the library run:
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

# Extensions
CPPEXT   = .c
CXXEXT   = .cpp
OBJEXT   = .o

# Input/Output directories

OBJECTS_DIR = ../obj
OUTPUT_DIR  = ../
SOURCE_DIR  = .

# Library name
TARGET=$(OUTPUT_DIR)/librp.so

# List of compiled object files
OBJECTS =	common.o \
			housekeeping.o \
			dpin_handler.o \
			analog_mixed_signals.o \
			apin_handler.o \
			rp.o

OBJS = $(patsubst %$(OBJEXT), $(OBJECTS_DIR)/%$(OBJEXT), $(OBJECTS))

# GCC compiling & linking flags
CFLAGS=-g -std=gnu99 -Wall -Werror -fPIC
CFLAGS += -DVERSION=$(VERSION) -DREVISION=$(REVISION)
LDFLAGS=-shared -Wl,--version-script=exportmap

# Red Pitaya common SW directory
SHARED=../../shared/

# Additional libraries which needs to be dynamically linked to the executable
# -lm - System math library (used by cos(), sin(), sqrt(), ... functions)
LIBS=-lm -lpthread

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
$(OBJECTS_DIR)/%.o:$(SOURCE_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $< -o $@

# Makefile target with rules how to link executable for each target from $(TARGET)
# list.
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

# Version header for traceability
version.h:
	cp $(SHARED)/include/redpitaya/version.h . 

# Clean target - when called it cleans all object files and executables.
clean:
	rm -f $(TARGET) $(OBJECTS_DIR)/*.o

# Install target - creates 'bin/' sub-directory in $(INSTALL_DIR) and copies all
# executables to that location.
install:
	mkdir -p $(INSTALL_DIR)/bin
	cp $(TARGET) $(INSTALL_DIR)/bin
