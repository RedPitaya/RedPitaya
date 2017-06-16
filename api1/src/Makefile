# (c) Red Pitaya  http://www.redpitaya.com
#
# Library librp project file. To build the library run:
# 'make all'

# Extensions
CPPEXT   = .c
OBJEXT   = .o

# Directories paths
OBJECTS_DIR = ../obj
INSTALL_DIR ?= .
SOURCE_DIR  = .
OUTPUT_DIR  = ../lib

SHARED_LIB=$(OUTPUT_DIR)/librp1.so
STATIC_LIB=$(OUTPUT_DIR)/librp1.a

# Library name
TARGET=$(SHARED_LIB) $(STATIC_LIB)

# List of compiled object files
OBJECTS = uio.o hwid.o evn.o

OBJS = $(patsubst %$(OBJEXT), $(OBJECTS_DIR)/%$(OBJEXT), $(OBJECTS))

# GCC compiling & linking flags
CFLAGS  = -std=gnu99 -Wall -Werror -fPIC -O3 -s
CFLAGS  = -std=gnu99 -Wall -fPIC -O0 -g
CFLAGS += -I../include
LDFLAGS=-shared

# Additional libraries which needs to be dynamically linked to the executable
# -lm - System math library (used by cos(), sin(), sqrt(), ... functions)
LIBS=-lm -ludev

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
	$(CC) -c $(CFLAGS) $< -o $@

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
	mkdir -p $(INSTALL_DIR)/include/api1
	cp  $(TARGET) $(INSTALL_DIR)/lib
	cp -r ../include/* $(INSTALL_DIR)/include
	cp -r *.h $(INSTALL_DIR)/include/api1

# TODO: should be removed later
main: $(TARGET) main.c
	$(CC) -g -o $@ main.c -lrp1 -L../lib
