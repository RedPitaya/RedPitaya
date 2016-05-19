#
# $Id: Makefile 2015-09-24 18:23:00Z Ulrich Habel (DF4IAH) $
#
# Red Pitaya specific application Makefile for RadioBox.
#

CROSS_COMPILE ?= arm-linux-gnueabihf-
CC=$(CROSS_COMPILE)gcc

OBJECTS=main.o worker.o cb_http.o cb_ws.o fpga_sys_xadc.o fpga_hk.o fpga_rb.o fpga.o calib.o rp_gain_compensation.o
CFLAGS+= -Wall -Werror -g -fPIC $(INCLUDE)
LDFLAGS= -shared -lpthread

OUT_DIR = ..
OUT_NAME ?= controllerhf.so
CONTROLLER = $(OUT_DIR)/$(OUT_NAME)

all: $(CONTROLLER)

$(CONTROLLER): $(OBJECTS)
	$(CC) -o $(CONTROLLER) $(OBJECTS) $(CFLAGS) $(LDFLAGS)

Test_AC97_Access: Test_AC97_Access.c
	$(CC) -o $@ $< $(CFLAGS)

clean:
	-$(RM) $(OBJECTS)
	-$(RM) -r img
