CXX=$(CROSS_COMPILE)g++
RM=rm

PROJECT_ROOT=../..

CXXSOURCES_WEB=main.cpp

INCLUDE =  -I$(INSTALL_DIR)/include
INCLUDE += -I$(INSTALL_DIR)/include/api2
INCLUDE += -I$(INSTALL_DIR)/include/apiApp
INCLUDE += -I$(INSTALL_DIR)/rp_sdk
INCLUDE += -I$(INSTALL_DIR)/rp_sdk/libjson

LIBS = -L$(INSTALL_DIR)/lib
LIBS += -L$(INSTALL_DIR)/rp_sdk

COMMON_FLAGS+=-Wall -fPIC -pedantic -Os -s -w -DASIO_STANDALONE
CFLAGS += $(COMMON_FLAGS) $(INCLUDE)
CXXFLAGS+=$(COMMON_FLAGS) -std=c++14 $(INCLUDE)
LDFLAGS = $(COMMON_FLAGS) $(LIBS)
LDWFLAGS+= -Wl,--whole-archive
LDWFLAGS+= -lrp_sdk -lrp-arb -lrp -lrp-hw -lrp-hw-calib -lrp-hw-profiles
LDWFLAGS+= -Wl,--no-whole-archive
LDWFLAGS+= -shared


COBJECTS=$(CSOURCES:.c=.o)
CXXOBJECTS=$(CXXSOURCES:.cpp=.o)
CXXOBJECTS_WEB=$(CXXSOURCES_WEB:.cpp=.o)
CXXOBJECTS_TEST=$(CXXSOURCES_TEST:.cpp=.o)
OBJECTS=$(COBJECTS) $(CXXOBJECTS)

INCLUDE += -I$(INSTALL_DIR)/include/api250-12
LDWFLAGS += -lrp-gpio -lrp-i2c -lrp-spi



OUT_DIR=../
C_OUT_NAME=$(OUT_DIR)controllerhf.so

.PHONY: all  make_c_app

all: make_c_app
clean: clean_c_app

make_c_app:  $(OBJECTS) $(CXXOBJECTS_WEB)
	$(CXX) -o $(C_OUT_NAME) $(OBJECTS) $(CXXOBJECTS_WEB) $(CFLAGS) $(LDFLAGS) $(LDWFLAGS)


clean_c_app:
	$(RM) -f $(C_OUT_NAME) $(OBJECTS) $(CXXOBJECTS_WEB)
