CXX=$(CROSS_COMPILE)g++
RM=rm

CXXSOURCES=main.cpp

RP_API=/root/redpitaya-public/api/lib
RP_SDK=/root/redpitaya-public/Bazaar/nginx/ngx_ext_modules/ws_server/rp_sdk

INCLUDE = -I$(RP_SDK)
INCLUDE+= -I/root/redpitaya-public/Applications/api/rpApplications/src -I/root/redpitaya-public/api/include
INCLUDE+= -I/root/redpitaya-public/Bazaar/tools/libjson

COMMON_FLAGS+=-Wall -fPIC -Os -s
CXXFLAGS+=$(COMMON_FLAGS) -std=c++11 $(INCLUDE)
LDFLAGS =-shared $(COMMON_FLAGS) -L/root/redpitaya-public/Applications/api/lib -L/root/redpitaya-public/api/lib
LDFLAGS+= -Wl,--whole-archive,--no-as-needed
LDFLAGS+= -lcryptopp
LDFLAGS+=-L$(RP_API) -lrpapp -lrp
LDFLAGS+=-L$(RP_SDK) -lrp_sdk
LDFLAGS+= -Wl,--no-whole-archive

CXXOBJECTS=$(CXXSOURCES:.cpp=.o)
OBJECTS=$(CXXOBJECTS)

OUT_DIR=../
C_OUT_NAME=$(OUT_DIR)controllerhf.so

all: make_c_app

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@

make_c_app: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(C_OUT_NAME) $(LDFLAGS)

clean:
	-$(RM) -f $(C_OUT_NAME) $(OBJECTS)