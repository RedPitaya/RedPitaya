CC=$(CROSS_COMPILE)gcc
CXX=$(CROSS_COMPILE)g++
RM=rm

CXXSOURCES= \
        new_main.cpp \

COBJECTS=$(CSOURCES:.c=.o) main.o dsp.o fpga.o spectrometerApp.o rpApp.o waterfall.o
CXXOBJECTS=$(CXXSOURCES:.cpp=.o)

OBJECTS=$(COBJECTS) $(CXXOBJECTS)

RP_API=../../../api-mockup/lib
RP_SDK=../../../Bazaar/nginx/ngx_ext_modules/ws_server/rp_sdk

INCLUDE = -I$(RP_SDK) -I$(RP_SDK)/../libjson
INCLUDE+= -I../../../api-mockup/rpApplications/src/ -I../../../api-mockup/rpbase/src/

INCLUDE+= -I$(RP_SDK)/include -I$(RP_SDK)/include/libjson

FFT_DIR=./external/kiss_fft
FFT_OBJECTS=$(FFT_DIR)/kiss_fft.o $(FFT_DIR)/kiss_fftr.o
FFT_INC=-I$(FFT_DIR)

JPEG_DIR=./external/jpeg-6b
JPEG_LIB=$(JPEG_DIR)/libjpeg.a
JPEG_INC=-I$(JPEG_DIR)

INCLUDE+=$(FFT_INC) $(JPEG_INC)

CFLAGS+= -Wall -fPIC $(INCLUDE)
CXXFLAGS+=$(COMMON_FLAGS) -std=c++11 $(INCLUDE) -fPIC
LDFLAGS=-shared $(COMMON_FLAGS) -L$(RP_API) -L$(RP_SDK) -Wl,--whole-archive -lrp_sdk -lrpapp -lrp -Wl,--no-whole-archive


OUT_DIR=../
C_OUT_NAME=$(OUT_DIR)controller.so

all: make_c_app
clean: clean_c_app clean_ext

$(JPEG_LIB):
	$(MAKE) -C $(JPEG_DIR)

$(FFT_OBJECTS):
	$(MAKE) -C $(FFT_DIR)

make_c_app: $(FFT_OBJECTS) $(JPEG_LIB) $(OBJECTS)
	$(CC) -o $(C_OUT_NAME) $(OBJECTS) $(FFT_OBJECTS) $(CFLAGS) $(LDFLAGS) $(JPEG_LIB)

clean_c_app:
	$(RM) -f $(C_OUT_NAME) $(OBJECTS)

clean_ext:
	$(MAKE) -C $(FFT_DIR) clean
	$(MAKE) -C $(JPEG_DIR) clean
