##
# (c) Red Pitaya  http://www.redpitaya.com
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

# List of compiled object files (not yet linked to executable)
OBJS = spectrum.o cli_parse_args.o
# List of raw source files (all object files, renamed from .o to .cpp)
SRCS = $(subst .o,.cpp, $(OBJS)))

# Executable name
TARGET = spectrum

# GCC compiling & linking flags
CXXFLAGS  = -std=c++14 -Wall -Wextra -Weffc++ -O3
CXXFLAGS += -I../../api/include -L../../api/lib
CXXFLAGS += -DVERSION=$(VERSION) -DREVISION=$(REVISION)

# Additional libraries which needs to be dynamically linked to the executable
LIBS = -lrp -pthread -static-libstdc++

# Main GCC executable (used for compiling and linking)
CXX = $(CROSS_COMPILE)g++
# Installation directory
INSTALL_DIR ?= .

ifeq ($(MODEL),Z20_250_12)
LIBS += -L$(INSTALL_DIR)/lib 
LIBS += -lrp-i2c
endif 

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
%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

# Makefile target with rules how to link executable for each target from $(TARGET)
# list.
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

# Clean target - when called it cleans all object files and executables.
clean:
	rm -f $(TARGET) $(OBJS)

# Install target - creates 'bin/' sub-directory in $(INSTALL_DIR) and copies all
# executables to that location.
install:
	mkdir -p $(INSTALL_DIR)/bin
	cp $(TARGET) $(INSTALL_DIR)/bin
