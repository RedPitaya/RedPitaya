
CXX=$(CROSS_COMPILE)g++
RM=rm

CXXSOURCES=main.cpp settings.cpp

INCLUDE =  -I$(INSTALL_DIR)/include
INCLUDE += -I$(INSTALL_DIR)/include/api2
INCLUDE += -I$(INSTALL_DIR)/include/apiApp
INCLUDE += -I$(INSTALL_DIR)/rp_sdk
INCLUDE += -I$(INSTALL_DIR)/rp_sdk/libjson

LIBS = -L$(INSTALL_DIR)/lib
LIBS += -L$(INSTALL_DIR)/rp_sdk

COMMON_FLAGS+=-Wall -fPIC -Os -s -w -mfpu=neon
CFLAGS+=$(COMMON_FLAGS) $(INCLUDE)
CXXFLAGS+=$(COMMON_FLAGS) -std=c++11 $(INCLUDE)
LDFLAGS = -shared $(COMMON_FLAGS) $(LIBS)
LDFLAGS+= -Wl,--whole-archive
LDFLAGS+= -lrp_sdk -lrpapp -lrpapp_lcr -lrp -lrp-hw -lrp-hw-calib -lrp-hw-profiles
LDFLAGS+= -Wl,--no-whole-archive

COBJECTS=$(CSOURCES:.c=.o)
CXXOBJECTS=$(CXXSOURCES:.cpp=.o)
OBJECTS=$(COBJECTS) $(CXXOBJECTS)

OUT_DIR=../
C_OUT_NAME=$(OUT_DIR)controllerhf.so
DIR=$(dir $(abspath $(firstword $(MAKEFILE_LIST))))

INCLUDE += -I$(INSTALL_DIR)/include/api250-12
LDFLAGS += -lrp-gpio -lrp-i2c -lrp-spi -lrp-hw


all: make_c_app
clean: clean_c_app

make_c_app: $(OBJECTS)
	$(CXX) -o $(C_OUT_NAME) $(OBJECTS) $(CFLAGS) $(LDFLAGS)

clean_c_app:
	$(RM) -f ba_api.*
	$(RM) -f $(C_OUT_NAME) $(OBJECTS)
