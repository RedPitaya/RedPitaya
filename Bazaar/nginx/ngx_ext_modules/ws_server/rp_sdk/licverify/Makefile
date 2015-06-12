CC=$(CROSS_COMPILE)g++
CFLAGS=-c -Wall -static -std=c++11 -Ibuild/include/cryptopp -I../../libjson -Iencoder
LDFLAGS= -Lbuild/lib -lcryptopp
SOURCES= main.cpp \
	LicenseVerificator.cpp \
	encoder/encoder.cpp \
	encoder/pubkey.cpp \
	encoder/privkey.cpp \
	../../libjson/_internal/Source/internalJSONNode.cpp \
	../../libjson/_internal/Source/JSONChildren.cpp ../../libjson/_internal/Source/JSONDebug.cpp \
	../../libjson/_internal/Source/JSONIterators.cpp ../../libjson/_internal/Source/JSONMemory.cpp \
	../../libjson/_internal/Source/JSONNode_Mutex.cpp ../../libjson/_internal/Source/JSONNode.cpp \
	../../libjson/_internal/Source/JSONWorker.cpp ../../libjson/_internal/Source/JSONWriter.cpp \
	../../libjson/_internal/Source/libjson.cpp ../../libjson/_internal/Source/JSONValidator.cpp \
	../../libjson/_internal/Source/JSONStream.cpp ../../libjson/_internal/Source/JSONAllocator.cpp \
    	../../libjson/_internal/Source/JSONPreparse.cpp \
	
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
