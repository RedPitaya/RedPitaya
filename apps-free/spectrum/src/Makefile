CC=$(CROSS_COMPILE)gcc
RM=rm

OBJECTS=main.o fpga.o worker.o dsp.o waterfall.o

FFT_DIR=./external/kiss_fft
FFT_OBJECTS=$(FFT_DIR)/kiss_fft.o $(FFT_DIR)/kiss_fftr.o
FFT_INC=-I$(FFT_DIR)

JPEG_DIR=./external/jpeg-6b
JPEG_LIB=$(JPEG_DIR)/libjpeg.a
JPEG_INC=-I$(JPEG_DIR)

INCLUDE=$(FFT_INC) $(JPEG_INC)

CFLAGS+= -Wall -Werror -g -fPIC $(INCLUDE)
LDFLAGS=-shared

CONTROLLER = ../controllerhf.so

all: $(CONTROLLER)

$(JPEG_LIB):
	$(MAKE) -C $(JPEG_DIR)

$(FFT_OBJECTS):
	$(MAKE) -C $(FFT_DIR)

$(CONTROLLER): $(FFT_OBJECTS) $(JPEG_LIB) $(OBJECTS)
	$(CC) -o $(CONTROLLER) $(OBJECTS) $(FFT_OBJECTS) $(CFLAGS) $(LDFLAGS) $(JPEG_LIB)

clean:
	$(RM) -f $(OBJECTS)
	$(MAKE) -C $(FFT_DIR) clean
	$(MAKE) -C $(JPEG_DIR) clean
