LIBJSON_DIR=../libjson
CC=$(CROSS_COMPILE)g++
CFLAGS=-c -Wall -static -std=c++11 -I../build/include/cryptopp -I$(LIBJSON_DIR) -I../encoder
LDFLAGS= -L../build/lib -lcryptopp
SOURCES= main.cpp \
	../encoder/encoder.cpp \
	../encoder/pubkey.cpp \
	../encoder/privkey.cpp \
	$(LIBJSON_DIR)/_internal/Source/internalJSONNode.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONChildren.cpp $(LIBJSON_DIR)/_internal/Source/JSONDebug.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONIterators.cpp $(LIBJSON_DIR)/_internal/Source/JSONMemory.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONNode_Mutex.cpp $(LIBJSON_DIR)/_internal/Source/JSONNode.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONWorker.cpp $(LIBJSON_DIR)/_internal/Source/JSONWriter.cpp \
	$(LIBJSON_DIR)/_internal/Source/libjson.cpp $(LIBJSON_DIR)/_internal/Source/JSONValidator.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONStream.cpp $(LIBJSON_DIR)/_internal/Source/JSONAllocator.cpp \
	$(LIBJSON_DIR)/_internal/Source/JSONPreparse.cpp \

	
OBJECTS=$(SOURCES:.cpp=.o)
CRYPTO_DIR=../cryptopp/
CRYPTO_INSTALL_DIR=../build/
CRYPTO_LIB=$(CRYPTO_INSTALL_DIR)/lib/libcryptopp.a
EXECUTABLE=idgen

all: $(CRYPTO_LIB) $(SOURCES) $(EXECUTABLE)

$(CRYPTO_LIB):
	mkdir -p $(CRYPTO_INSTALL_DIR)
	make -C $(CRYPTO_DIR) CXX=${CROSS_COMPILE}g++ PREFIX=../build static
	make -C $(CRYPTO_DIR) CXX=${CROSS_COMPILE}g++ PREFIX=../build install

$(EXECUTABLE):	$(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

install:
	test -d '$(DESTDIR)/sbin' || mkdir -p '$(DESTDIR)/sbin'
	cp $(EXECUTABLE) '$(DESTDIR)/sbin'

clean: 
	rm -rf $(EXECUTABLE) $(OBJECTS) $(CRYPTO_LIB) 
	make -C $(CRYPTO_DIR) CXX=${CROSS_COMPILE}g++ PREFIX=../build clean
	make -C $(CRYPTO_DIR) CXX=${CROSS_COMPILE}g++ PREFIX=../build remove
