
CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -pedantic -Wno-psabi -Wno-unused-parameter
LDFLAGS := -L. -lstreaming_api
INCLUDES := -I.

PRGS = adc_example_1.exe \
       adc_example_2.exe \
       dac_example_1.exe \
       dac_example_2.exe \
       dac_example_3.exe

OBJS := $(patsubst %,%.o,$(basename $(PRGS)))
SRC := $(patsubst %,%.cpp,$(basename $(PRGS)))

all: $(PRGS)

$(PRGS): %.exe: %.cpp
	$(CXX) $< $(CXXFLAGS) $(LDFLAGS) $(INCLUDES) -o $@

clean:
	del /Q *.o 2>nul

clean_all: clean
	del /Q $(PRGS) 2>nul

.PHONY: all clean clean_all