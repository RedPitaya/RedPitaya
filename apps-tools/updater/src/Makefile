CXX=$(CROSS_COMPILE)g++
RM=rm

CXXSOURCES=main.cpp

COMMON_FLAGS+=-Wall -fPIC -lstdc++ -Os -s
CXXFLAGS+=$(COMMON_FLAGS) -std=c++11 $(INCLUDE)

CXXOBJECTS=$(CXXSOURCES:.cpp=.o)
OBJECTS=$(CXXOBJECTS)

OUT_DIR=../
C_OUT_NAME=$(OUT_DIR)daemon

all: $(SOURCES) make_c_app

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@

make_c_app: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(C_OUT_NAME) $(LDFLAGS)

clean:
	-$(RM) -f $(C_OUT_NAME) $(OBJECTS)
