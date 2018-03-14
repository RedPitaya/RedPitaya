CC=$(CROSS_COMPILE)gcc
RM=rm

CSOURCES=main.c

INCLUDE =  -I$(INSTALL_DIR)/include
INCLUDE += -I$(INSTALL_DIR)/include/api2
INCLUDE += -I$(INSTALL_DIR)/include/apiApp
INCLUDE += -I$(INSTALL_DIR)/rp_sdk
INCLUDE += -I$(INSTALL_DIR)/rp_sdk/libjson

LIBS = -L$(INSTALL_DIR)/lib
LIBS += -L$(INSTALL_DIR)/rp_sdk

COMMON_FLAGS+=-Wall -fPIC -Os -s -funroll-loops
CFLAGS+=$(COMMON_FLAGS) $(INCLUDE)
ifeq ($(DISABLE_ASSERTS), true)
CFLAGS+= -DNDEBUG
endif
LDFLAGS =-shared $(COMMON_FLAGS) $(LIBS)
LDFLAGS+= -Wl,--whole-archive
LDFLAGS+= -lcryptopp -lrp_sdk -lrp2
LDFLAGS+= -Wl,--no-whole-archive

COBJECTS=$(CSOURCES:.c=.o)
OBJECTS=$(COBJECTS)

OUT_DIR=../
C_OUT_NAME=$(OUT_DIR)controllerhf.so

all: $(SOURCES) make_c_app

.cpp.o:
	$(CC) -c $(CFLAGS) $< -o $@

make_c_app: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(C_OUT_NAME) $(LDFLAGS)

clean:
	-$(RM) -f $(C_OUT_NAME) $(OBJECTS)
