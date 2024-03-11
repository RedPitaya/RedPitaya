
CXX=$(CROSS_COMPILE)g++
RM=rm

CXXSOURCES=main.cpp
CXXSOURCES_TOOL=lcr.cpp

INCLUDE =  -I$(INSTALL_DIR)/include
INCLUDE += -I$(INSTALL_DIR)/include/api2
INCLUDE += -I$(INSTALL_DIR)/include/apiApp
INCLUDE += -I$(INSTALL_DIR)/rp_sdk
INCLUDE += -I$(INSTALL_DIR)/rp_sdk/libjson

LIBS = -L$(INSTALL_DIR)/lib
LIBS += -L$(INSTALL_DIR)/rp_sdk

COMMON_FLAGS+=-Wall -fPIC -Os -s
CXXFLAGS+=$(COMMON_FLAGS) -std=c++11 $(INCLUDE)
LDFLAGS = -shared  $(COMMON_FLAGS) $(LIBS)
LDFLAGS+= -Wl,--whole-archive,--no-as-needed
LDFLAGS+= -lcryptopp -lrpapp_lcr -lrp -lrp_sdk
LDFLAGS+= -Wl,--no-whole-archive

LDFLAGS_TOOL = $(COMMON_FLAGS) $(LIBS)
LDFLAGS_TOOL+= -Wl,--whole-archive,--no-as-needed
LDFLAGS_TOOL+= -static -lpthread -lrpapp_lcr -lrp
LDFLAGS_TOOL+= -Wl,--no-whole-archive


INCLUDE += -I$(INSTALL_DIR)/include/api250-12
LDFLAGS += -lrp-gpio -lrp-i2c -lrp-spi
LDFLAGS_TOOL += -lrp-gpio -lrp-i2c -lrp-spi -lrp-hw  -lrp-hw-calib -lrp-hw-profiles -li2c

CXXOBJECTS=$(CXXSOURCES:.cpp=.o)
OBJECTS=$(CXXOBJECTS)

CXXOBJECTS_TOOL=$(CXXSOURCES_TOOL:.cpp=.o)
OBJECTS_TOOL=$(CXXOBJECTS_TOOL)


OUT_DIR=../
C_OUT_NAME=$(OUT_DIR)controllerhf.so
C_OUT_NAME_TOOL=$(OUT_DIR)lcr

all: make_c_app

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@

make_c_app: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(C_OUT_NAME) $(LDFLAGS)

lcr: $(OBJECTS_TOOL)
	$(CXX) $(OBJECTS_TOOL) -o $(C_OUT_NAME_TOOL) $(LDFLAGS_TOOL)

clean:
	-$(RM) -f $(C_OUT_NAME) $(OBJECTS)
	-$(RM) -f $(C_OUT_NAME_TOOL) $(OBJECTS_TOOL)
