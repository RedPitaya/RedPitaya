##
# $Id: $
#
# (c) Red Pitaya  http://www.redpitaya.com
#
# scpi-server project file. To build the server run:
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

# Directories paths
OBJECTS_DIR = ../obj
OUTPUT_DIR  = ../
SOURCE_DIR  = .
INSTALL_DIR ?= .
MODEL ?= Z10

# Library name
TARGET=$(OUTPUT_DIR)/scpi-server

# List of compiled object files
OBJECTS =	scpi-commands.o \
		scpi-server.o \
		api_cmd.o \
		dpin.o \
		apin.o \
		acquire.o \
		generate.o \
		common.o

OBJS = $(patsubst %$(OBJEXT), $(OBJECTS_DIR)/%$(OBJEXT), $(OBJECTS))

# GCC compiling & linking flags
CFLAGS += -g -std=gnu99 -Wall -Werror -fPIC -D$(MODEL)
CFLAGS += -I$(INSTALL_DIR)/include
CFLAGS += -DVERSION=$(VERSION) -DREVISION=$(REVISION)
LDFLAGS=

# Additional libraries which needs to be dynamically linked to the executable
# -lm - System math library (used by cos(), sin(), sqrt(), ... functions)
LIBPATH= -L ../scpi-parser/libscpi/dist -L $(INSTALL_DIR)/lib
LIBS= -lm -lpthread -lrp -l:libscpi.a

INC= -I../scpi-parser/libscpi/inc -I$(INSTALL_DIR)/include

ifeq ($(MODEL),Z20_250_12)
INC += -I$(INSTALL_DIR)/include/api250-12
LIBS += -lrp-gpio -lrp-i2c -lrp-spi
endif 

# Main GCC executable (used for compiling and linking)
CC=$(CROSS_COMPILE)gcc

# Main Makefile target 'all' - it iterates over all targets listed in $(TARGET)
# variable.
all: $(TARGET)

# Target with compilation rules to compile object from source files.
# It applies to all files ending with .o. During partial building only new object
# files are created for the source files (.c) which have newer timestamp then 
# objects (.o) files.
$(OBJECTS_DIR)/%.o:$(SOURCE_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

# Makefile target with rules how to link executable for each target from $(TARGET)
# list.
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBPATH) $(LIBS)

# Clean target - when called it cleans all object files and executables.
clean:
	rm -f $(TARGET) $(OBJECTS_DIR)/*.o

# Install target - creates 'bin/' sub-directory in $(INSTALL_DIR) and copies all
# executables to that location.
install:
	mkdir -p $(INSTALL_DIR)/bin
	cp $(TARGET) $(INSTALL_DIR)/bin
