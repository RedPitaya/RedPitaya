
CC=$(CROSS_COMPILE)gcc
CXX=$(CROSS_COMPILE)g++
RM=rm

CXXSOURCES=main.cpp settings.cpp sig_gen.cpp common.cpp sig_gen_logic.cpp osc_logic.cpp math_logic.cpp

INCLUDE =  -I$(INSTALL_DIR)/include
INCLUDE += -I$(INSTALL_DIR)/include/api2
INCLUDE += -I$(INSTALL_DIR)/include/apiApp
INCLUDE += -I$(INSTALL_DIR)/include/redpitaya
INCLUDE += -I$(INSTALL_DIR)/rp_sdk
INCLUDE += -I$(INSTALL_DIR)/rp_sdk/libjson

LIBS = -L$(INSTALL_DIR)/lib
LIBS += -L$(INSTALL_DIR)/rp_sdk

COMMON_FLAGS+=-Wall -fPIC
COMMON_FLAGS+=-Os -s
#COMMON_FLAGS+=-g

CXXFLAGS+=$(COMMON_FLAGS) -std=c++11 $(INCLUDE)  -D_FILE_OFFSET_BITS=64
LDFLAGS = -shared $(COMMON_FLAGS) $(LIBS)
#LDFLAGS+= -Wl,--whole-archive,--no-as-needed
LDFLAGS+= -lcryptopp -lrpapp -lrp-arb -lrp-sweep -lrp -lrp_sdk  -lrp-hw -lrp-hw-calib -lrp-hw-profiles
#LDFLAGS+= -Wl,--no-whole-archive

LDFLAGS += -lrp-gpio -lrp-i2c -lrp-spi -lrp-formatter


CXXOBJECTS=$(CXXSOURCES:.cpp=.o)
OBJECTS=$(CXXOBJECTS)

OUT_DIR=../
C_OUT_NAME=$(OUT_DIR)controllerhf.so

.PHONY: all make_c_app

all: make_c_app

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@

make_c_app: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(C_OUT_NAME) $(LDFLAGS)

clean:
	-$(RM) -f $(C_OUT_NAME) $(OBJECTS)
