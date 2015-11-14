CXX=$(CROSS_COMPILE)g++
RM=rm

CXXSOURCES=main.cpp

RP_API=../../../api/lib
RP_SDK=../../../Bazaar/nginx/ngx_ext_modules/ws_server/rp_sdk

INCLUDE = -I$(RP_SDK)
INCLUDE+= -I../../../api/rpApplications/lcr_meter/src -I../../../api/include
INCLUDE+= -I../../../Bazaar/tools -I../../../Bazaar/tools/libjson

COMMON_FLAGS+=-Wall -fPIC -lstdc++ -Os -s
CXXFLAGS+=$(COMMON_FLAGS) -std=c++11 $(INCLUDE)
LDFLAGS =-shared $(COMMON_FLAGS) -L$(RP_SDK)/lib
LDFLAGS+= -Wl,--whole-archive
LDFLAGS+=-L$(RP_SDK) -lrp_sdk
LDFLAGS+=-L$(RP_API) -lrpapp_lcr -lrp
LDFLAGS+= -Wl,--no-whole-archive

CXXOBJECTS=$(CXXSOURCES:.cpp=.o)
OBJECTS=$(CXXOBJECTS)

OUT_DIR=../
C_OUT_NAME=$(OUT_DIR)controllerhf.so

all: $(SOURCES) make_c_app

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@

make_c_app: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(C_OUT_NAME) $(LDFLAGS)

clean:
	-$(RM) -f $(C_OUT_NAME) $(OBJECTS)
