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
DECODERS_DIR=../../../../../Applications/la_pro/src/

CXX=$(CROSS_COMPILE)g++
CXXFLAGS=-c -Wall -Os -static -std=c++11 -fPIC -I$(LIBJSON_DIR) -DNDEBUG -I../../../../tools -I$(DECODERS_DIR) -I.

ifeq ($(ALWAYS_PURCHASED),true)
CXXFLAGS+=-DALWAYS_PURCHASED
endif

ifeq ($(ENABLE_LICENSING),true)
PRO_DIR=../../../../../Applications
SOURCES+= $(PRO_DIR)/licverify/LicenseVerificator.cpp \
          $(PRO_DIR)/licverify/encoder/encoder.cpp \
          $(PRO_DIR)/licverify/encoder/pubkey.cpp \
          $(PRO_DIR)/licverify/encoder/privkey.cpp
CXXFLAGS+=-I$(PRO_DIR)/licverify -I. -DENABLE_LICENSING
endif

ifeq ($(DIGITAL_LOOP),true)
CXXFLAGS+=-DIGITAL_LOOP
endif

OBJECTS=$(patsubst %.cpp,$(SDKOBJDIR)/%.o, $(SOURCES))

LIB=librp_sdk.a

all: $(SOURCES) $(LIB)

$(LIB): $(OBJECTS)
	ar rc $(LIB) $(OBJECTS)

$(SDKOBJDIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -rf $(LIB) $(OBJDIR)
