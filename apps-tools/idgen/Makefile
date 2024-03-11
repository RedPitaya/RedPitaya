PRO_DIR=../../Applications
LIBJSON_DIR=../../Bazaar/tools/libjson
CXX=$(CROSS_COMPILE)g++
CXXFLAGS=-c -Wall -static -std=c++11 -I$(LIBJSON_DIR) -I$(PRO_DIR)/licverify/ -I../
LDFLAGS= -L../build/lib -lcryptopp
SOURCES= main.cpp \
	$(PRO_DIR)/licverify/encoder/encoder.cpp \
	$(PRO_DIR)/licverify/encoder/pubkey.cpp \
	$(PRO_DIR)/licverify/encoder/privkey.cpp \
	$(LIBJSON_DIR)/_internal/Source/internalJSONNode.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONChildren.cpp $(LIBJSON_DIR)/_internal/Source/JSONDebug.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONIterators.cpp $(LIBJSON_DIR)/_internal/Source/JSONMemory.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONNode_Mutex.cpp $(LIBJSON_DIR)/_internal/Source/JSONNode.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONWorker.cpp $(LIBJSON_DIR)/_internal/Source/JSONWriter.cpp \
	$(LIBJSON_DIR)/_internal/Source/libjson.cpp $(LIBJSON_DIR)/_internal/Source/JSONValidator.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONStream.cpp $(LIBJSON_DIR)/_internal/Source/JSONAllocator.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONPreparse.cpp \

	
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=idgen

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE):	$(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) $< -o $@

install:
	test -d '$(DESTDIR)/sbin' || mkdir -p '$(DESTDIR)/sbin'
	cp $(EXECUTABLE) '$(DESTDIR)/sbin'

clean: 
	rm -rf $(EXECUTABLE) $(OBJECTS) 
