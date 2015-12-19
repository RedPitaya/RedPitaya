LIBJSON_DIR=../../../tools/libjson
CXX=$(CROSS_COMPILE)g++
CXXFLAGS=-c -Wall -O3 -static -std=c++11 -Iwebsocketpp -I$(SYSROOT)/usr/include -I$(LIBJSON_DIR) -I$(LIBJSON_DIR)/.. -L$(SYSROOT)/usr/lib/ -L. -lboost_system -DWEBSOCKETPP_STRICT_MASKING
SOURCES= rp_websocket_server.cpp \
	ws_server.cpp \
	$(LIBJSON_DIR)/_internal/Source/internalJSONNode.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONChildren.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONDebug.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONIterators.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONMemory.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONNode_Mutex.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONNode.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONWorker.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONWriter.cpp \
	$(LIBJSON_DIR)/_internal/Source/libjson.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONValidator.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONStream.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONAllocator.cpp \
    	$(LIBJSON_DIR)/_internal/Source/JSONPreparse.cpp \

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
