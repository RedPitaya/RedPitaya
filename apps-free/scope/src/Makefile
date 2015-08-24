CROSS_COMPILE ?= arm-linux-gnueabihf-
CC=$(CROSS_COMPILE)gcc
RM=rm

OBJECTS=main.o fpga.o worker.o calib.o fpga_awg.o generate.o fpga_pid.o pid.o

CFLAGS+= -Wall -Werror -g -fPIC $(INCLUDE)
LDFLAGS=-shared

OUT_DIR = ../
OUT_NAME ?= controllerhf.so
CONTROLLER = $(OUT_DIR)/$(OUT_NAME)

all: $(CONTROLLER)

$(CONTROLLER): $(OBJECTS)
	$(CC) -o $(CONTROLLER) $(OBJECTS) $(CFLAGS) $(LDFLAGS)

clean:
	-$(RM) -f $(OBJECTS)
