CC=$(CROSS_COMPILE)gcc
CXX=$(CROSS_COMPILE)g++
RM=rm

CXXSOURCES= \
	main.cpp \

RP_API=../../../api-mockup/lib
RP_SDK=../../../Bazaar/nginx/ngx_ext_modules/ws_server/rp_sdk

INCLUDE = -I$(RP_SDK)
INCLUDE+= -I../../../api-mockup/rpApplications/src/ -I../../../api-mockup/rpbase/src/
INCLUDE = -I../../../Bazaar/tools -I../../../Bazaar/tools/libjson

#CFLAGS=$(CFLAGS) -Wall -Werror -g
COMMON_FLAGS+=-Wall -g -fPIC -O0
CFLAGS+=$(COMMON_FLAGS) $(INCLUDE)
CXXFLAGS+=$(COMMON_FLAGS) -std=c++11 $(INCLUDE)
LDFLAGS =-shared $(COMMON_FLAGS) -L$(RP_SDK)/lib
LDFLAGS+= -Wl,--whole-archive
LDFLAGS+=-L$(RP_SDK) -lrp_sdk
LDFLAGS+=-L$(RP_API) -lrpapp -lrp
LDFLAGS+= -Wl,--no-whole-archive

CXXOBJECTS=$(CXXSOURCES:.cpp=.o)

OBJECTS=$(CXXOBJECTS)

OUT_DIR=../
C_OUT_NAME=$(OUT_DIR)controller.so

all: $(SOURCES) make_c_app

clean: clean_c_app

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

make_c_app: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(C_OUT_NAME) $(LDFLAGS)

clean_c_app:
	$(RM) -f $(C_OUT_NAME) $(OBJECTS)
