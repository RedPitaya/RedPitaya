MODEL ?= Z10
STREAMING_MODE ?= MASTER
MODE ?= Release

CXX=$(CROSS_COMPILE)g++
RM=rm

PROJECT_ROOT=../..

CXXSOURCES=main.cpp

CXXSOURCES_SERVICE = console_server/main.cpp \
					 console_server/options.cpp \
					 console_server/streaming.cpp \
 					 console_server/dac_streaming.cpp

INCLUDE =  -I$(INSTALL_DIR)/include
INCLUDE += -I$(INSTALL_DIR)/include/api2
INCLUDE += -I$(INSTALL_DIR)/include/apiApp
INCLUDE += -I$(INSTALL_DIR)/rp_sdk
INCLUDE += -I$(INSTALL_DIR)/rp_sdk/libjson

INCLUDE += -I./server/libs/src/
INCLUDE += -I./server/libs/src/common
INCLUDE += -I./server/common
INCLUDE += -I./server/libs/asio/include


LIBS = -L$(INSTALL_DIR)/lib
LIBS += -L$(INSTALL_DIR)/rp_sdk
LIBS += -L./server/bin
LIBS += -L./server/lib
LIBS += -L./server/libs


COMMON_FLAGS+= -Wall -fPIC -Os -s -w -DASIO_STANDALONE -DASDASD
CFLAGS += $(COMMON_FLAGS) $(INCLUDE) -D$(MODEL)
CXXFLAGS+=$(COMMON_FLAGS) -std=c++17 $(INCLUDE) -D$(MODEL)
LDFLAGS = $(COMMON_FLAGS) $(LIBS)
LDWFLAGS+= -Wl,--whole-archive
LDWFLAGS+= -ljsoncpp -lrpsasrv
LDWFLAGS+= -lrp_sdk -lrp
LDWFLAGS+= -Wl,--no-whole-archive
LDWFLAGS+= -shared

LDSFLAGS+= -lrpsasrv -ljsoncpp
LDSFLAGS+= -lpthread
LDSFLAGS+= -lrp -static-libstdc++ -static-libgcc

ifeq ($(MODEL),Z20_250_12)
INCLUDE += -I$(INSTALL_DIR)/include/api250-12
LDFLAGS += -lrp-gpio -lrp-i2c -lrp-spi
endif

ifeq ($(STREAMING_MODE),MASTER)
CXXFLAGS+= -DSTREAMING_MASTER
endif

ifeq ($(STREAMING_MODE),SLAVE)
CXXFLAGS+= -DSTREAMING_SLAVE
endif

COBJECTS=$(CSOURCES:.c=.o)
CXXOBJECTS=$(CXXSOURCES:.cpp=.o)
OBJECTS=$(COBJECTS) $(CXXOBJECTS)

CXXOBJECTS_SERVICE=$(CXXSOURCES_SERVICE:.cpp=.o)
OBJECTS_SERVICE=$(COBJECTS) $(CXXOBJECTS_SERVICE)

OUT_DIR=../
C_OUT_NAME=$(OUT_DIR)controllerhf.so
C_OUT_NAME_SERVICE=$(OUT_DIR)streaming-server

.PHONY: all

all: make_c_app service
clean: clean_c_app

build_lib:
	./build.sh libs -DCMAKE_BUILD_TYPE=$(MODE) -DMODEL=$(MODEL)

.PHONY: make_c_app
make_c_app: build_lib  $(OBJECTS)
	$(CXX) -o $(C_OUT_NAME) $(OBJECTS) $(CXXFLAGS) $(LDFLAGS) $(LDWFLAGS)

.PHONY: service
service: build_lib $(OBJECTS_SERVICE)
	$(CXX) -o $(C_OUT_NAME_SERVICE) $(OBJECTS_SERVICE) $(CXXFLAGS) $(LDFLAGS) $(LDSFLAGS)

clean_c_app:
	$(RM) -f $(C_OUT_NAME) $(OBJECTS) $(OBJECTS_SERVICE) $(C_OUT_NAME_SERVICE)
	./build.sh clean
