
CC=$(CROSS_COMPILE)gcc
CXX=$(CROSS_COMPILE)g++
RM=rm

CXXSOURCES = main.cpp settings.cpp  rp_math.cpp sweepController.cpp common.cpp

INCLUDE =  -I$(INSTALL_DIR)/include
INCLUDE += -I$(INSTALL_DIR)/include/api2
INCLUDE += -I$(INSTALL_DIR)/include/apiApp
INCLUDE += -I$(INSTALL_DIR)/rp_sdk
INCLUDE += -I$(INSTALL_DIR)/rp_sdk/libjson

LIBS = -L$(INSTALL_DIR)/lib
LIBS += -L$(INSTALL_DIR)/rp_sdk

COMMON_FLAGS+=-Wall -fPIC -Os -s

CFLAGS+=$(COMMON_FLAGS) $(INCLUDE)
CXXFLAGS+=$(COMMON_FLAGS) -std=c++11 $(INCLUDE)  -D_FILE_OFFSET_BITS=64
LDFLAGS += -shared $(COMMON_FLAGS) $(LIBS)
#LDFLAGS += -Wl,--whole-archive,--no-as-needed
LDFLAGS += -lcryptopp -lrpapp -lrp -lrp_sdk -lrp-dsp -lrp-hw -lrp-hw-calib -lrp-hw-profiles
#LDFLAGS += -Wl,--no-whole-archive

INCLUDE += -I$(INSTALL_DIR)/include/api250-12
LDFLAGS += -lrp-gpio -lrp-i2c -lrp-spi

COBJECTS=$(CSOURCES:.c=.o)
CXXOBJECTS=$(CXXSOURCES:.cpp=.o)
OBJECTS=$(COBJECTS) $(CXXOBJECTS)

OUT_DIR=../
C_OUT_NAME=$(OUT_DIR)controllerhf.so

all: make_c_app

make_c_app: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(C_OUT_NAME) $(CFLAGS) $(LDFLAGS)

clean:
	-$(RM) -f $(C_OUT_NAME) $(OBJECTS)
