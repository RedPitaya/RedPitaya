CXX=arm-xilinx-linux-gnueabi-g++
CXXFLAGS=-c -Wall -O3 -static -std=c++11 -fPIC -I../libjson
SOURCES= DataManager.cpp \
	../libjson/_internal/Source/internalJSONNode.cpp \
	../libjson/_internal/Source/JSONChildren.cpp \
	../libjson/_internal/Source/JSONDebug.cpp \
	../libjson/_internal/Source/JSONIterators.cpp \
	../libjson/_internal/Source/JSONMemory.cpp \
	../libjson/_internal/Source/JSONNode_Mutex.cpp \
	../libjson/_internal/Source/JSONNode.cpp \
	../libjson/_internal/Source/JSONWorker.cpp \
	../libjson/_internal/Source/JSONWriter.cpp \
	../libjson/_internal/Source/libjson.cpp \
	../libjson/_internal/Source/JSONValidator.cpp \
	../libjson/_internal/Source/JSONStream.cpp \
	../libjson/_internal/Source/JSONAllocator.cpp \
	../libjson/_internal/Source/JSONPreparse.cpp \
	
OBJDIR=./objs/rp_sdk
OBJECTS=$(patsubst %.cpp,$(OBJDIR)/%.o, $(SOURCES))

LIB=librp_sdk.a

all: $(SOURCES) $(LIB)
    
$(LIB): $(OBJECTS) 
	ar rc $(LIB) $(OBJECTS)
	rm ../../../../../Applications/rp_sdk/lib/*.a
	cp librp_sdk.a ../../../../../Applications/rp_sdk/lib
	rm ../../../../../Applications/rp_sdk/include/*.h
	cp *.h ../../../../../Applications/rp_sdk/include

$(OBJDIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $< -o $@

