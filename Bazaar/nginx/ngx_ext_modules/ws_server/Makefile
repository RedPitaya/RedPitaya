SYSROOT=../../../../OS/buildroot/buildroot-2014.02/output/host/usr/armeb-buildroot-linux-gnueabi/sysroot
CXX=$(CROSS_COMPILE)g++
CXXFLAGS=-c -Wall -O3 -static -std=c++11 -Iwebsocketpp -I$(SYSROOT)/usr/include -L$(SYSROOT)/usr/lib/ -L. -lboost_system
SOURCES= rp_websocket_server.cpp \
	ws_server.cpp \
	libjson/_internal/Source/internalJSONNode.cpp \
	libjson/_internal/Source/JSONChildren.cpp \
	libjson/_internal/Source/JSONDebug.cpp \
	libjson/_internal/Source/JSONIterators.cpp \
	libjson/_internal/Source/JSONMemory.cpp \
	libjson/_internal/Source/JSONNode_Mutex.cpp \
	libjson/_internal/Source/JSONNode.cpp \
	libjson/_internal/Source/JSONWorker.cpp \
	libjson/_internal/Source/JSONWriter.cpp \
	libjson/_internal/Source/libjson.cpp \
	libjson/_internal/Source/JSONValidator.cpp \
	libjson/_internal/Source/JSONStream.cpp \
	libjson/_internal/Source/JSONAllocator.cpp \
    	libjson/_internal/Source/JSONPreparse.cpp \
	
OBJECTS=$(SOURCES:.cpp=.o)
LIB=libws_server.a

RP_MANAGER_DIR=./rp_sdk
RP_MANAGER_LIB=$(RP_MANAGER_DIR)/librp_sdk.a

all: $(RP_MANAGER_LIB) $(SOURCES) $(LIB)
    
$(LIB): $(OBJECTS) 
	ar rc $(LIB) $(OBJECTS)

.cpp.o:
	$(CXX) $(CXXFLAGS) $< -o $@

$(RP_MANAGER_LIB): 
	cd $(RP_MANAGER_DIR); $(MAKE)

clean: 
	rm -rf $(LIB) $(OBJECTS)
	$(MAKE) -C $(RP_MANAGER_DIR) clean
