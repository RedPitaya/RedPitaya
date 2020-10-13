MODEL ?= Z10

CXX=$(CROSS_COMPILE)g++
RM=rm

PROJECT_ROOT=../..

CXXSOURCES=main.cpp acq.cpp

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

COBJECTS=$(CSOURCES:.c=.o)
CXXOBJECTS=$(CXXSOURCES:.cpp=.o)
OBJECTS=$(COBJECTS) $(CXXOBJECTS)


OUT_DIR=../
C_OUT_NAME=$(OUT_DIR)controllerhf.so

.PHONY: all

all: make_c_app 
clean: clean_c_app

.PHONY: make_c_app
make_c_app:  $(OBJECTS)
	$(CXX) -o $(C_OUT_NAME) $(OBJECTS) $(CFLAGS) $(LDFLAGS) $(LDWFLAGS)

clean_c_app:
	$(RM) -f $(C_OUT_NAME) $(OBJECTS)
