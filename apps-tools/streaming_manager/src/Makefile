MODEL ?= Z10

CXX=$(CROSS_COMPILE)g++
RM=rm

PROJECT_ROOT=../..

CXXSOURCES=main.cpp

INCLUDE =  -I$(INSTALL_DIR)/include
INCLUDE += -I$(INSTALL_DIR)/include/api2
INCLUDE += -I$(INSTALL_DIR)/include/apiApp
INCLUDE += -I$(INSTALL_DIR)/rp_sdk
INCLUDE += -I$(INSTALL_DIR)/rp_sdk/libjson

INCLUDE += -I./server/include/rpsa/server/core
INCLUDE += -I./server/include/rpsa/common/core
INCLUDE += -I./server/common
INCLUDE += -I./server/libs/asio/include


LIBS = -L$(INSTALL_DIR)/lib
LIBS += -L$(INSTALL_DIR)/rp_sdk
LIBS += -L./server/bin

COMMON_FLAGS+=-Wall -fPIC -Os -s -w -DASIO_STANDALONE -DASDASD
CFLAGS += $(COMMON_FLAGS) $(INCLUDE)
CXXFLAGS+=$(COMMON_FLAGS) -std=c++11 $(INCLUDE) -D$(MODEL)
LDFLAGS = -shared $(COMMON_FLAGS) $(LIBS)
LDFLAGS+= -Wl,--whole-archive
LDFLAGS+= -lrp_sdk -lrp
LDFLAGS+= -lrpsasrv
LDFLAGS+= -Wl,--no-whole-archive

COBJECTS=$(CSOURCES:.c=.o)
CXXOBJECTS=$(CXXSOURCES:.cpp=.o)
OBJECTS=$(COBJECTS) $(CXXOBJECTS)

OUT_DIR=../
C_OUT_NAME=$(OUT_DIR)controllerhf.so


all: make_c_app
clean: clean_c_app

.PHONY: build_lib
build_lib:
	./build.sh libs

.PHONY: make_c_app
make_c_app: build_lib  $(OBJECTS)
	$(CXX) -o $(C_OUT_NAME) $(OBJECTS) $(CFLAGS) $(LDFLAGS)

clean_c_app:
	$(RM) -f $(C_OUT_NAME) $(OBJECTS)
	./build.sh clean
