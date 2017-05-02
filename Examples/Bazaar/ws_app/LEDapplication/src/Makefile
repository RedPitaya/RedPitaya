CXX=$(CROSS_COMPILE)g++
RM=rm

CXXSOURCES=main.cpp


RP_SDK=../rp_sdk

INCLUDE = -I$(RP_SDK)
INCLUDE+= -I../libjson


COMMON_FLAGS+=-Wall -fPIC -lstdc++ -Os -s -DNDEBUG -Wno-reorder
CXXFLAGS+=$(COMMON_FLAGS) -std=c++11 $(INCLUDE)
#LDFLAGS =-shared $(COMMON_FLAGS)
LDFLAGS =-shared $(COMMON_FLAGS) -L$(RP_SDK)
LDFLAGS+= -Wl,--whole-archive
LDFLAGS+=-L$(RP_SDK) -lrp_sdk
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
