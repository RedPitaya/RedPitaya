CC=$(CROSS_COMPILE)gcc
CXX=$(CROSS_COMPILE)g++
RM=rm

CSOURCES= \
	main.c \
	fpga.c \
	worker.c \
	calib.c \
	fpga_awg.c \
	generate.c \
	fpga_pid.c \
	pid.c \

CXXSOURCES= \
	new_main.cpp \

RP_SDK=../../../Bazaar/nginx/ngx_ext_modules/ws_server/rp_sdk

INCLUDE= -I$(RP_SDK) -I../../../Bazaar/nginx/ngx_ext_modules/ws_server/libjson

#CFLAGS=$(CFLAGS) -Wall -Werror -g
COMMON_FLAGS+=-Wall -g -fPIC -O0
CFLAGS+=$(COMMON_FLAGS) $(INCLUDE)
CXXFLAGS+=$(COMMON_FLAGS) -std=c++11 $(INCLUDE)
LDFLAGS=-shared $(COMMON_FLAGS) -L$(RP_SDK)/lib -Wl,--whole-archive -lrp_sdk -lcryptopp -Wl,--no-whole-archive

COBJECTS=$(CSOURCES:.c=.o)
CXXOBJECTS=$(CXXSOURCES:.cpp=.o)

OBJECTS=$(COBJECTS) $(CXXOBJECTS)

OUT_DIR=../
C_OUT_NAME=$(OUT_DIR)controller.so

all: $(SOURCES) make_c_app

clean: clean_c_app

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

make_c_app: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(C_OUT_NAME) $(LDFLAGS)

clean_c_app:
	$(RM) -f $(C_OUT_NAME) $(OBJECTS)

