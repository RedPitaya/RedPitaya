CXX := g++
CXXFLAGS :=  -std=c++20 -Wall -Wextra -pedantic -Wno-psabi -Wno-unused-parameter
LDFLAGS := -L. -lstreaming_api -Wl,-rpath='$$ORIGIN'
INCLUDES := -I.

PRGS =  adc_example_1 \
		adc_example_2 \
		dac_example_1 \
		dac_example_2 \
		dac_example_3

OBJS := $(patsubst %,%.o,$(PRGS))
SRC := $(patsubst %,%.cpp,$(PRGS))


all: $(PRGS)

$(PRGS): %: %.cpp
	$(CXX) $< $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) -o $@

clean:
	$(RM) *.o
	$(RM) $(OBJS)

clean_all: clean
	$(RM) $(PRGS)
