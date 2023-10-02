

CXX=$(CROSS_COMPILE)g++
RM=rm

CXXSOURCES=main.cpp i2c_decoder.cpp spi_decoder.cpp uart_decoder.cpp can_decoder.cpp

INCLUDE =  -I$(INSTALL_DIR)/include
INCLUDE += -I$(INSTALL_DIR)/include/api2
INCLUDE += -I$(INSTALL_DIR)/include/apiApp
INCLUDE += -I$(INSTALL_DIR)/rp_sdk
INCLUDE += -I$(INSTALL_DIR)/rp_sdk/libjson

LIBS = -L$(INSTALL_DIR)/lib
LIBS += -L$(INSTALL_DIR)/rp_sdk

COMMON_FLAGS+=-Wall -fPIC -lstdc++ -Os -s -funroll-loops -std=c++11
CXXFLAGS+=$(COMMON_FLAGS) -std=c++11 $(INCLUDE)
ifeq ($(DISABLE_ASSERTS),true)
CXXFLAGS+= -DNDEBUG
endif
LDFLAGS =-shared $(COMMON_FLAGS) $(LIBS)
LDFLAGS+= -Wl,--whole-archive
LDFLAGS+= -lcryptopp -lrp_sdk -lrp2
LDFLAGS+= -Wl,--no-whole-archive

INCLUDE += -I$(INSTALL_DIR)/include/api250-12
LDFLAGS += -lrp-gpio -lrp-i2c -lrp-hw  -lrp-hw-calib -lrp-hw-profiles

CXXOBJECTS=$(CXXSOURCES:.cpp=.o)
OBJECTS=$(CXXOBJECTS)

OUT_DIR=../
C_OUT_NAME=$(OUT_DIR)controllerhf.so

.PHONY: all

all: make_c_app

# .cpp.o:
# 	$(CXX) -c  $< -o $@

make_c_app: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(C_OUT_NAME) $(LDFLAGS) $(CXXFLAGS)

clean:
	-$(RM) -f $(C_OUT_NAME) $(OBJECTS)
