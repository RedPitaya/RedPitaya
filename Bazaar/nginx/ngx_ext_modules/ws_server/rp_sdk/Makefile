LIBJSON_DIR=../../../../tools/libjson
SOURCES= DataManager.cpp \
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
	$(LIBJSON_DIR)/_internal/Source/JSONPreparse.cpp

OBJDIR=./objs
SDKOBJDIR=$(OBJDIR)/rp_sdk

CRYPTO_DIR=../../../../tools/cryptopp
CRYPTO_INSTALL_DIR=../../../../tools/build

CXX=$(CROSS_COMPILE)g++
CXXFLAGS=-c -Wall -O0 -static -std=c++11 -fPIC -I$(LIBJSON_DIR) -I$(CRYPTO_INSTALL_DIR)/include/cryptopp -DNDEBUG
ifeq ($(ALWAYS_PURCHASED),true)
CXXFLAGS+=-DALWAYS_PURCHASED
endif

ifeq ($(ENABLE_LICENSING),true)
SOURCES+= licverify/LicenseVerificator.cpp licverify/encoder/encoder.cpp licverify/encoder/pubkey.cpp licverify/encoder/privkey.cpp
CXXFLAGS+=-Ilicverify/encoder -DENABLE_LICENSING
endif

ifeq ($(DIGITAL_LOOP),true)
CXXFLAGS+=-DIGITAL_LOOP
endif

OBJECTS=$(patsubst %.cpp,$(SDKOBJDIR)/%.o, $(SOURCES))

LIB=librp_sdk.a
CRYPTO_LIB=$(CRYPTO_INSTALL_DIR)/lib/libcryptopp.a

CURRENT_DIR := $(shell pwd)

all: $(CRYPTO_LIB) $(SOURCES) $(LIB)

$(CRYPTO_LIB):
	mkdir -p $(CRYPTO_INSTALL_DIR)
	make -C $(CRYPTO_DIR) CXX=${CXX} PREFIX=../build static
	make -C $(CRYPTO_DIR) CXX=${CXX} PREFIX=../build install

$(LIB): $(OBJECTS)
	mkdir -p $(OBJDIR)/cryptopp
	cd $(OBJDIR)/cryptopp; ar -x $(CURRENT_DIR)/$(CRYPTO_LIB)
	ar rc $(LIB) objs/cryptopp/*.o $(OBJECTS)

$(SDKOBJDIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -rf $(LIB) $(OBJDIR)
	make -C $(CRYPTO_DIR) CXX=${CXX} PREFIX=../build clean
	make -C $(CRYPTO_DIR) CXX=${CXX} PREFIX=../build remove
