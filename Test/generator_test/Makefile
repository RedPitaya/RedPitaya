TARGET=generator_test

CC=$(CROSS_COMPILE)gcc
CXX=$(CROSS_COMPILE)g++

CXXSOURCES=main.cpp

RP_BASE_API=../../api/lib
RP_API=../../Applications/api/lib

INCLUDE  = -I../../Applications/api/rpApplications/lcr_meter/src -I../../../api/include
INCLUDE += -I../../Applications/api/rpApplications/src/ 
INCLUDE += -I../../api/include

COMMON_FLAGS += -Wall -O3 -funroll-loops -fopenmp
CFLAGS += $(COMMON_FLAGS) $(INCLUDE)
CXXFLAGS += $(COMMON_FLAGS) -std=gnu++11 $(INCLUDE)
LDFLAGS += -Wl,--whole-archive
LDFLAGS += -L$(RP_BASE_API) -L$(RP_API) -lrpapp -lrpapp_lcr -lrp
LDFLAGS += -Wl,--no-whole-archive

COBJECTS=$(CSOURCES:.c=.o)
CXXOBJECTS=$(CXXSOURCES:.cpp=.o)
OBJECTS=$(COBJECTS) $(CXXOBJECTS)


all: $(OBJECTS)
	$(CXX) -o $(TARGET) $(OBJECTS) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJECTS)
