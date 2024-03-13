INSTALL_DIR ?= /opt/redpitaya
CFLAGS  = -std=gnu11 -Wall ## -Werror
CFLAGS += -I$(INSTALL_DIR)/include
LDFLAGS = -L$(INSTALL_DIR)/lib
LDLIBS = -static -lrp-dsp -lrp -lrp-hw-calib -lrp-hw-profiles

INCLUDE += -I$(INSTALL_DIR)/include/api250-12
LDLIBS += -lrp-gpio -lrp-i2c
LDLIBS += -lrp-hw -lm -lstdc++ -lpthread -li2c

CXXFLAGS  = -std=c++17 -Wall ## -Werror
CXXFLAGS += -I$(INSTALL_DIR)/include


# List of compiled object files (not yet linked to executable)

PRGS =  trig_pos_test

PRGS_PP = acq_trigger_test dsp_test

OBJS := $(patsubst %,%.o,$(PRGS))
SRC := $(patsubst %,%.c,$(PRGS))

OBJS_PP := $(patsubst %,%.o,$(PRGS_PP))
SRC_PP := $(patsubst %,%.cpp,$(PRGS_PP))

all: $(PRGS) $(PRGS_PP)

$(PRGS): %: %.c
	$(CC) $< $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $@

$(PRGS_PP): %: %.cpp
	g++ $< $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) -o $@

install:
	mkdir -p $(INSTALL_DIR)/tests
	cp $(PRGS) $(INSTALL_DIR)/tests
	cp $(PRGS_PP) $(INSTALL_DIR)/tests


clean:
	$(RM) *.o
	$(RM) $(OBJS)
	$(RM) $(OBJS_PP)
	$(RM) $(PRGS)
	$(RM) $(PRGS_PP)
