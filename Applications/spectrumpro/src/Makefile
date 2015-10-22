CC=$(CROSS_COMPILE)gcc
CXX=$(CROSS_COMPILE)g++
RM=rm

CXXSOURCES=main.cpp

RP_API=../../../api/lib
RP_SDK=../../../Bazaar/nginx/ngx_ext_modules/ws_server/rp_sdk

INCLUDE = -I$(RP_SDK) -I$(RP_SDK)/../../../../tools/libjson
INCLUDE+= -I../../../api/rpApplications/src/ -I../../../api/rpbase/src/
INCLUDE+= -I../../../Bazaar/tools -I../../../Bazaar/tools/libjson

JPEG_DIR=./external/jpeg-6b
JPEG_LIB=$(JPEG_DIR)/libjpeg.a
JPEG_INC=-I$(JPEG_DIR)

INCLUDE+=$(JPEG_INC)

COMMON_FLAGS+=-Wall -fPIC -Os -s
CFLAGS+=$(COMMON_FLAGS) $(INCLUDE)
CXXFLAGS+=$(COMMON_FLAGS) -std=c++11 $(INCLUDE)
LDFLAGS =-shared $(COMMON_FLAGS) -L$(RP_SDK)/lib
LDFLAGS+= -Wl,--whole-archive
LDFLAGS+=-L$(RP_SDK) -lrp_sdk
LDFLAGS+=-L$(RP_API) -lrpapp -lrp
LDFLAGS+= -Wl,--no-whole-archive

COBJECTS=$(CSOURCES:.c=.o) waterfall.o
CXXOBJECTS=$(CXXSOURCES:.cpp=.o)
OBJECTS=$(COBJECTS) $(CXXOBJECTS)

OUT_DIR=../
C_OUT_NAME=$(OUT_DIR)controllerhf.so

all: make_c_app
clean: clean_c_app clean_ext

$(JPEG_LIB):
	$(MAKE) -C $(JPEG_DIR)

make_c_app: $(JPEG_LIB) $(OBJECTS)
	$(CXX) -o $(C_OUT_NAME) $(OBJECTS) $(CFLAGS) $(LDFLAGS) $(JPEG_LIB)

clean_c_app:
	$(RM) -f $(C_OUT_NAME) $(OBJECTS)

clean_ext:
	$(MAKE) -C $(JPEG_DIR) clean
