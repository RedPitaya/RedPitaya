CFLAGS  = -std=gnu11 -Wall ## -Werror
CFLAGS += -I/opt/redpitaya/include
LDFLAGS = -L/opt/redpitaya/lib
LDLIBS = -static -lrp-hw-can -lrp -lrp-hw-calib -lrp-hw-profiles

INCLUDE += -I/opt/redpitaya/include/api250-12
LDLIBS += -lrp-gpio -lrp-i2c
LDLIBS += -lrp-hw -lm -lstdc++ -lpthread -li2c -lsocketcan


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
		uart_loopback \
		can_pass \
		can_loopback \
		calibration_api

PRGS += acquire_trigger_software \
		acquire_trigger_posedge \
		acquire_signal_check \
		generate_arbitrary_waveform \
		generate_burst_trigger_external \
		generate_burst_trigger_software \
		generate_continuous \
		generate_two_burst_trigger_software \
		generate_two_trigger_software_sync \
		axi \
		axi_4ch

PRGS += i2c_eeprom_read_calib
PRGS += acquire_4ch_trigger_software
PRGS += i2c_switch_max7311

OBJS := $(patsubst %,%.o,$(PRGS))
SRC := $(patsubst %,%.c,$(PRGS))

all: $(PRGS)

$(PRGS): %: %.c
	$(CC) $< $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $@

clean:
	$(RM) *.o
	$(RM) $(OBJS)

clean_all: clean
	$(RM) $(PRGS)

