LIBJSON_DIR=../../../../../tools/libjson
CC=$(CROSS_COMPILE)g++
CFLAGS=-c -Wall -static -std=c++11 -Ibuild/include/cryptopp -I$(LIBJSON_DIR) -Iencoder
LDFLAGS= -Lbuild/lib -lcryptopp
SOURCES= main.cpp \
	LicenseVerificator.cpp \
	encoder/encoder.cpp \
	encoder/pubkey.cpp \
	encoder/privkey.cpp \
	$(LIBJSON_DIR)/_internal/Source/internalJSONNode.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONChildren.cpp $(LIBJSON_DIR)/_internal/Source/JSONDebug.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONIterators.cpp $(LIBJSON_DIR)/_internal/Source/JSONMemory.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONNode_Mutex.cpp $(LIBJSON_DIR)/_internal/Source/JSONNode.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONWorker.cpp $(LIBJSON_DIR)/_internal/Source/JSONWriter.cpp \
	$(LIBJSON_DIR)/_internal/Source/libjson.cpp $(LIBJSON_DIR)/_internal/Source/JSONValidator.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONStream.cpp $(LIBJSON_DIR)/_internal/Source/JSONAllocator.cpp \
    	$(LIBJSON_DIR)/_internal/Source/JSONPreparse.cpp \
	
OBJECTS=$(SOURCES:.cpp=.o)
CRYPTO_DIR=cryptopp/
CRYPTO_INSTALL_DIR=build/
CRYPTO_LIB=$(CRYPTO_INSTALL_DIR)/lib/libcryptopp.a

EXECUTABLE=licver

all: $(CRYPTO_LIB) $(SOURCES) $(EXECUTABLE)

$(CRYPTO_LIB):
	mkdir -p $(CRYPTO_INSTALL_DIR)
	make -C $(CRYPTO_DIR) CXX=${CROSS_COMPILE}g++ PREFIX=../build static
	make -C $(CRYPTO_DIR) CXX=${CROSS_COMPILE}g++ PREFIX=../build install

$(EXECUTABLE):	$(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean: 
	rm -rf $(EXECUTABLE) $(OBJECTS)
	make -C $(CRYPTO_DIR) CXX=${CROSS_COMPILE}g++ PREFIX=../build clean
