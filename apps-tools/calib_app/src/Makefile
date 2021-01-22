MODEL ?= Z10

CXX=$(CROSS_COMPILE)g++
RM=rm

PROJECT_ROOT=../..

CXXSOURCES=acq.cpp calib.cpp calib_man.cpp acq_math.cpp filter_logic2ch.cpp
CXXSOURCES_WEB=main.cpp
CXXSOURCES_TEST=test.cpp

ifeq ($(MODEL),$(filter $(MODEL),Z10 Z20_125))
CXXSOURCES += filter_logic.cpp
endif

INCLUDE =  -I$(INSTALL_DIR)/include
INCLUDE += -I$(INSTALL_DIR)/include/api2
INCLUDE += -I$(INSTALL_DIR)/include/apiApp
INCLUDE += -I$(INSTALL_DIR)/rp_sdk
INCLUDE += -I$(INSTALL_DIR)/rp_sdk/libjson

LIBS = -L$(INSTALL_DIR)/lib
LIBS += -L$(INSTALL_DIR)/rp_sdk

COMMON_FLAGS+=-Wall -fPIC -Os -s -w -DASIO_STANDALONE
CFLAGS += $(COMMON_FLAGS) $(INCLUDE)
CXXFLAGS+=$(COMMON_FLAGS) -std=c++14 $(INCLUDE) -D$(MODEL)
LDFLAGS = $(COMMON_FLAGS) $(LIBS)
LDWFLAGS+= -Wl,--whole-archive
LDWFLAGS+= -lrp_sdk -lrp
LDWFLAGS+= -Wl,--no-whole-archive
LDWFLAGS+= -shared

LDWFLAGS_TEST+= -Wl,--whole-archive
LDWFLAGS_TEST+=  -static -lpthread -lrp
LDWFLAGS_TEST+= -Wl,--no-whole-archive


COBJECTS=$(CSOURCES:.c=.o)
CXXOBJECTS=$(CXXSOURCES:.cpp=.o)
CXXOBJECTS_WEB=$(CXXSOURCES_WEB:.cpp=.o)
CXXOBJECTS_TEST=$(CXXSOURCES_TEST:.cpp=.o)
OBJECTS=$(COBJECTS) $(CXXOBJECTS)

ifeq ($(MODEL),Z20_250_12)
INCLUDE += -I$(INSTALL_DIR)/include/api250-12
LDWFLAGS += -lrp-gpio -lrp-i2c -lrp-spi
endif 


OUT_DIR=../
C_OUT_NAME=$(OUT_DIR)controllerhf.so
C_OUT_TEST=test.app

.PHONY: all test make_c_app

all: make_c_app
clean: clean_c_app

make_c_app:  $(OBJECTS) $(CXXOBJECTS_WEB)
	$(CXX) -o $(C_OUT_NAME) $(OBJECTS) $(CXXOBJECTS_WEB) $(CFLAGS) $(LDFLAGS) $(LDWFLAGS)

test: $(OBJECTS) $(CXXOBJECTS_TEST)
	$(CXX) -o $(C_OUT_TEST) $(OBJECTS) $(CXXOBJECTS_TEST) $(CFLAGS) $(LDFLAGS) $(LDWFLAGS_TEST)

clean_c_app:
	$(RM) -f $(C_OUT_NAME) $(OBJECTS) $(CXXOBJECTS_WEB) $(CXXOBJECTS_TEST) $(C_OUT_TEST)
