MODEL ?= Z10

CFLAGS  = -std=gnu11 -Wall ## -Werror
CFLAGS += -I/opt/redpitaya/include -D$(MODEL)
LDFLAGS = -L/opt/redpitaya/lib
LDLIBS = -static -lrp

ifeq ($(MODEL),Z20_250_12)
INCLUDE += -I/opt/redpitaya/include/api250-12
LDLIBS += -lrp-gpio -lrp-i2c
endif

LDLIBS += -lrp-hw -lm -lstdc++ -lpthread


# List of compiled object files (not yet linked to executable)

PRGS =  analog_inputs \
		analog_outputs \
		digital_led_bar \
		digital_led_blink \
		digital_push_button \
		i2c \
		spi \
		spi_loopback \
		test_e_module \
		uart \
		uart_loopback

ifeq ($(MODEL),$(filter $(MODEL),Z10 Z20_125 Z20 Z20_250_12))
PRGS += acquire_trigger_software \
		acquire_trigger_posedge \
		acquire_signal_check \
		generate_arbitrary_waveform \
		generate_burst_trigger_external \
		generate_burst_trigger_software \
		generate_continuous \
		generate_two_burst_trigger_software \
		generate_two_trigger_software_sync
endif

ifeq ($(MODEL),$(filter $(MODEL),Z10 Z20_125))
PRGS += i2c_eeprom_read_calib_125_14
endif

ifeq ($(MODEL),Z20_125_4CH)
PRGS += acquire_4ch_trigger_software
endif

ifeq ($(MODEL),Z20_250_12)
PRGS += i2c_switch_max7311
endif

OBJS := $(patsubst %,%.o,$(PRGS))
SRC := $(patsubst %,%.c,$(PRGS))

all: $(PRGS)

$(PRGS): %: %.c
	$(CC) $< $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $@

clean:
	$(RM) *.o
	$(RM) $(OBJS)
