##
# $Id: $
#
# (c) Red Pitaya  http://www.redpitaya.com
#
# Library librp project file. To build the library run:
# 'make all'
#
# This project file is written for GNU/Make software. For more details please 
# visit: http://www.gnu.org/software/make/manual/make.html
# GNU Compiler Collection (GCC) tools are used for the compilation and linkage. 
# For the details about the usage and building please visit:
# http://gcc.gnu.org/onlinedocs/gcc/
#

MODEL ?= Z10

# Versioning system
VERSION ?= 0.00-0000
REVISION ?= devbuild

# Extensions
CPPEXT   = .c
CXXEXT   = .cpp
OBJEXT   = .o

# Directories paths
OBJECTS_DIR = ../obj
INSTALL_DIR ?= .
SOURCE_DIR  = .
OUTPUT_DIR  = ../lib

SHARED_LIB=$(OUTPUT_DIR)/librp.so
STATIC_LIB=$(OUTPUT_DIR)/librp.a

# Library name
TARGET=$(SHARED_LIB) $(STATIC_LIB)

# List of compiled object files
OBJECTS =	common.o \
		kiss_fft/kiss_fft.o \
		kiss_fft/kiss_fftr.o \
		oscilloscope.o \
		acq_handler.o \
		generate.o \
		gen_handler.o \
		spec_dsp.o \
		spec_fpga.o \
		rp.o \
		hw/uart.o
		
ifeq ($(MODEL),Z20_250_12)
	OBJECTS += calib_250_12.o
else
	OBJECTS += calib.o
endif
OBJS = $(patsubst %$(OBJEXT), $(OBJECTS_DIR)/%$(OBJEXT), $(OBJECTS))

# GCC compiling & linking flags
CFLAGS  = -std=c11 -Wall -Werror -fPIC -Ikiss_fft -Os -s -D$(MODEL)
CFLAGS += -DVERSION=$(VERSION) -DREVISION=$(REVISION)
CFLAGS += -I../include
LDFLAGS=-shared -Wl,--version-script=exportmap

# Additional libraries which needs to be dynamically linked to the executable
# -lm - System math library (used by cos(), sin(), sqrt(), ... functions)
LIBS=-lm -lpthread

ifeq ($(MODEL),Z20_250_12)
INCLUDE += -I$(INSTALL_DIR)/include/api250-12
LDFLAGS += -L$(INSTALL_DIR)/lib 
LDFLAGS += -lrp-i2c
endif 

# Main GCC executable (used for compiling and linking)
CC=$(CROSS_COMPILE)gcc

AR=$(CROSS_COMPILE)ar

# Main Makefile target 'all' - it iterates over all targets listed in $(TARGET)
# variable.
all: $(TARGET)

# Target with compilation rules to compile object from source files.
# It applies to all files ending with .o. During partial building only new object
# files are created for the source files (.c) which have newer timestamp then 
# objects (.o) files.
$(OBJECTS_DIR)/%.o:$(SOURCE_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

# Makefile target with rules how to link executable for each target from
# $(SHARED_LIB) list.
$(SHARED_LIB): $(OBJS)
	mkdir -p $(OUTPUT_DIR)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) $(LDFLAGS)

$(STATIC_LIB): $(OBJS)
	mkdir -p $(OUTPUT_DIR)
	$(AR) rcs $@ $^

# Clean target - when called it cleans all object files and executables.
clean:
	rm -f $(TARGET) $(OBJECTS_DIR)/*.o

# Install target - creates 'bin/' sub-directory in $(INSTALL_DIR) and copies all
# executables to that location.
install:
	mkdir -p $(INSTALL_DIR)/lib
	mkdir -p $(INSTALL_DIR)/include
	cp  $(TARGET) $(INSTALL_DIR)/lib
	cp ../include/redpitaya/version.h $(INSTALL_DIR)/include/version.h
ifeq ($(MODEL),Z10)
	cp ../include/redpitaya/rp.h $(INSTALL_DIR)/include/rp.h
endif
ifeq ($(MODEL),Z20)
	cp ../include/redpitaya/rp_Z20.h $(INSTALL_DIR)/include/rp.h
endif
ifeq ($(MODEL),Z20_125)
	cp ../include/redpitaya/rp_Z20_125.h $(INSTALL_DIR)/include/rp.h
endif
ifeq ($(MODEL),Z20_250_12)
	cp ../include/redpitaya/rp_Z20_250_12.h $(INSTALL_DIR)/include/rp.h
endif
