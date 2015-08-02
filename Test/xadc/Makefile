CC=$(CROSS_COMPILE)gcc

# Executable name
TARGET=xadc

# GCC compiling & linking flags
CFLAGS=-g -std=gnu99 -Wall
CFLAGS += -DVERSION=$(VERSION) -DREVISION=$(REVISION)
CFLAGS += -Ixadc_lib/include
CFLAGS += -Ixadc_lib/src
CFLAGS += -Ixadc_lib/linux_include
#CFLAGS += -I../../tmp/linux-xlnx-xilinx-v2015.2.01/include

LIBS=-lm -lpthread

# List of compiled object files (not yet linked to executable)
OBJS = xadc_lib/src/xadc_core.c xadc_read_cmd/src/xadc_main.c
# List of raw source files (all object files, renamed from .o to .c)
SRCS = $(subst .o,.c, $(OBJS)))

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) #$(LIBS)
