CXX=$(CROSS_COMPILE)g++
RM=rm
CP=cp

CXXSOURCES=XMLUTF8Buffer.cpp  XMLAttribute.cpp  XMLCommon.cpp  XMLDocument.cpp  XMLNameSpace.cpp  XMLNode.cpp  XMLParser.cpp  XMLReader.cpp  XMLString.cpp XMLBuffer.cpp

COMMON_FLAGS+=-Wall -fPIC -lstdc++ -Os -s -Wno-sign-compare
CXXFLAGS+=$(COMMON_FLAGS) -std=c++14 $(INCLUDE)

CXXOBJECTS=$(CXXSOURCES:.cpp=.o)
OBJECTS=$(CXXOBJECTS)

OUT_DIR=../../bin/lib/
OUT_DIR_INC=$(OUT_DIR)include/
C_OUT_NAME=$(OUT_DIR)libxml.a

all: $(SOURCES) make_c_app

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@

make_c_app: $(OBJECTS)
	-mkdir -p $(OUT_DIR_INC)
	ar rcs $(C_OUT_NAME) $(OBJECTS)
	-$(CP) XML*.h $(OUT_DIR)/include/
clean:
	-$(RM) -rf $(C_OUT_NAME) $(OBJECTS) $(OUT_DIR)
