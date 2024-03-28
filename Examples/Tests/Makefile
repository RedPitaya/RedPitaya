INSTALL_DIR ?= /opt/redpitaya
CFLAGS  = -std=gnu11 -Wall ## -Werror
CFLAGS += -I$(INSTALL_DIR)/include
LDFLAGS = -L$(INSTALL_DIR)/lib
LDLIBS = -static -lrp-dsp -lrp -lrp-hw -lrp-hw-calib -lrp-hw-profiles

INCLUDE += -I$(INSTALL_DIR)/include/api250-12
LDLIBS += -lrp-gpio -lrp-i2c
LDLIBS += -lrp-hw -lm -lstdc++ -lpthread -li2c

CXXFLAGS  = -std=c++17 -Wall ## -Werror
CXXFLAGS += -I$(INSTALL_DIR)/include

SCPI_LIBRARY = libscpi.a
COMMON_LIBRARY = libcommon.a

# List of compiled object files (not yet linked to executable)

PRGS =  trig_pos_test

PRGS_PP = acq_trigger_test dsp_test acq_trigger_test_4ch
SCPI_PP = scpi/scpi_client scpi/socket
COMMON_PP = common/common

OBJS := $(patsubst %,%.o,$(PRGS))
SRC := $(patsubst %,%.c,$(PRGS))

OBJS_PP := $(patsubst %,%.o,$(PRGS_PP))
SRC_PP := $(patsubst %,%.cpp,$(PRGS_PP))

SCPI_OBJS_PP := $(patsubst %,%.o,$(SCPI_PP))
SCPI_SRC_PP := $(patsubst %,%.cpp,$(SCPI_PP))

COMMON_OBJS_PP := $(patsubst %,%.o,$(COMMON_PP))
COMMON_SRC_PP := $(patsubst %,%.cpp,$(COMMON_PP))

all: $(PRGS) $(PRGS_PP)

$(PRGS): %: %.c
	$(CC) $< $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $@

$(PRGS_PP): %: %.cpp $(SCPI_LIBRARY) $(COMMON_LIBRARY)
	g++ $< $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) $(SCPI_LIBRARY) $(COMMON_LIBRARY) -o $@

$(SCPI_PP):  %: %.cpp
	g++ $(CXXFLAGS) -c $< -o $@.o

$(SCPI_LIBRARY): $(SCPI_PP)
	ar rcs $@ $(SCPI_OBJS_PP)

$(COMMON_PP):  %: %.cpp
	g++ $(CXXFLAGS) -c $< -o $@.o

$(COMMON_LIBRARY): $(COMMON_PP)
	ar rcs $@ $(COMMON_OBJS_PP)

install:
	mkdir -p $(INSTALL_DIR)/tests
	cp $(PRGS) $(INSTALL_DIR)/tests
	cp $(PRGS_PP) $(INSTALL_DIR)/tests


clean:
	$(RM) *.o
	$(RM) $(SCPI_OBJS_PP)
	$(RM) $(COMMON_OBJS_PP)
	$(RM) $(OBJS_PP)
	$(RM) $(PRGS)
	$(RM) $(PRGS_PP)
	$(RM) $(SCPI_LIBRARY)
	$(RM) $(COMMON_LIBRARY)
