CROSS_COMPILE ?= arm-linux-gnueabihf-
CC=$(CROSS_COMPILE)gcc
RM=rm

OBJECTS=main.o fpga.o worker.o calib.o fpga_awg.o generate.o

#CFLAGS=$(CFLAGS) -Wall -Werror -g
CFLAGS+= -Wall -Werror -g -fPIC $(INCLUDE)
LDFLAGS= -shared

OUT_NAME   ?= controller.so
OUT_DIR    = ..
C_OUT_NAME = $(OUT_DIR)/$(OUT_NAME)

all: make_c_app
clean: clean_c_app

make_c_app: $(OBJECTS)
	$(CC) -o $(C_OUT_NAME) $(OBJECTS) $(CFLAGS) $(LDFLAGS)

clean_c_app:
	$(RM) -f $(OBJECTS)
