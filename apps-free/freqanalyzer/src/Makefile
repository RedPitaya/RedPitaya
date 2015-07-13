CC=$(CROSS_COMPILE)gcc
RM=rm

OBJECTS=main.o fpga.o worker.o dsp.o

FFT_DIR=./external/kiss_fft
FFT_OBJECTS=$(FFT_DIR)/kiss_fft.o $(FFT_DIR)/kiss_fftr.o
FFT_INC=-I$(FFT_DIR)

INCLUDE=$(FFT_INC)

CFLAGS+= -Wall -Werror -g -fPIC $(INCLUDE)
LDFLAGS=-shared


OUT_DIR		= ../
OUT_NAME	?= controllerhf.so
C_OUT_NAME	= $(OUT_DIR)/$(OUT_NAME)

all: make_c_app
clean: clean_c_app clean_ext

$(FFT_OBJECTS):
	$(MAKE) -C $(FFT_DIR)

make_c_app: $(FFT_OBJECTS) $(OBJECTS)
	$(CC) -o $(C_OUT_NAME) $(OBJECTS) $(FFT_OBJECTS) $(CFLAGS) $(LDFLAGS)

clean_c_app:
	$(RM) -f $(OBJECTS)

clean_ext:
	$(MAKE) -C $(FFT_DIR) clean

