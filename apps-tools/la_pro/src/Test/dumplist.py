uart_file = []
spi_file = []
i2c_file = []


# All modes for UART with non-inverted logic levels
uart_file.append({"file": "data/uart/8n1/hello_world_8n1_1200.sr", "params": "1200 625000 lsb 8 none 0 1 10 no"})
uart_file.append({"file": "data/uart/8n1/hello_world_8n1_2400.sr", "params": "2400 625000 lsb 8 none 0 1 10 no"})
uart_file.append({"file": "data/uart/8n1/hello_world_8n1_4800.sr", "params": "4800 625000 lsb 8 none 0 1 10 no"})
uart_file.append({"file": "data/uart/8n1/hello_world_8n1_9600.sr", "params": "9600 625000 lsb 8 none 0 1 10 no"})
uart_file.append({"file": "data/uart/8n1/hello_world_8n1_19200.sr", "params": "19200 1000000 lsb 8 none 0 1 10 no"})
uart_file.append({"file": "data/uart/8n1/hello_world_8n1_38400.sr", "params": "38400 1000000 lsb 8 none 0 1 10 no"})
uart_file.append({"file": "data/uart/8n1/hello_world_8n1_57600.sr", "params": "57600 1000000 lsb 8 none 0 1 10 no"})
uart_file.append({"file": "data/uart/8n1/hello_world_8n1_115200.sr", "params": "115200 1000000 lsb 8 none 0 1 10 no"})
uart_file.append({"file": "data/uart/8n1/hello_world_8n1_230400.sr", "params": "230400 5000000 lsb 8 none 0 1 10 no"})
uart_file.append({"file": "data/uart/8n1/hello_world_8n1_460800.sr", "params": "460800 5000000 lsb 8 none 0 1 10 no"})
uart_file.append({"file": "data/uart/8n1/hello_world_8n1_921600.sr", "params": "921600 5000000 lsb 8 none 0 1 10 no"})
uart_file.append({"file": "data/uart/8o1/hello_world_8o1_115200.sr", "params": "115200 1000000 lsb 8 odd 0 1 10 no"})
uart_file.append({"file": "data/uart/8e1/hello_world_8e1_115200.sr", "params": "115200 1000000 lsb 8 even 0 1 10 no"})
uart_file.append({"file": "data/uart/7o1/hello_world_7o1_115200.sr", "params": "115200 1000000 lsb 7 odd 0 1 10 no"})
uart_file.append({"file": "data/uart/7e1/hello_world_7e1_115200.sr", "params": "115200 1000000 lsb 7 even 0 1 10 no"})
# Some inverted logic files for uart:
uart_file.append({"file": "data/uart/8n1/inverted_hello_world_8n1_9600.sr", "params": "9600 625000 lsb 8 none 1 1 10 yes"})
uart_file.append({"file": "data/uart/8n1/inverted_hello_world_8n1_19200.sr", "params": "19200 1000000 lsb 8 none 1 1 10 yes"})
uart_file.append({"file": "data/uart/8n1/inverted_hello_world_8n1_38400.sr", "params": "38400 1000000 lsb 8 none 1 1 10 yes"})


# All CPOL and CPHA combinations
spi_file.append({"file": "data/spi/spi_atmega32_00.sr", "params": "1 3 2 low 0 0 msb 0 2 1 active-low 0 0 msb-first"})
spi_file.append({"file": "data/spi/spi_atmega32_01.sr", "params": "1 3 2 low 0 1 msb 0 2 1 active-low 0 1 msb-first"})
spi_file.append({"file": "data/spi/spi_atmega32_10.sr", "params": "1 3 2 low 1 0 msb 0 2 1 active-low 1 0 msb-first"})
spi_file.append({"file": "data/spi/spi_atmega32_11.sr", "params": "1 3 2 low 1 1 msb 0 2 1 active-low 1 1 msb-first"})
# Some other SPI dumps
spi_file.append({"file": "data/spi/spi_0x5a6b7c8d9e_cpol0_cpha1_trigger_cs_falling_lsbfirst_ok.sr", "params": "6 5 3 low 0 1 msb CS# CLK MOSI active-low 0 1 msb-first"})
spi_file.append({"file": "data/spi/spi_0x5a6b_cpol0_cpha1_trigger_cs_falling_ok.sr", "params": "6 5 3 low 0 1 msb CS# CLK MOSI active-low 0 1 msb-first"})
spi_file.append({"file": "data/spi/spi_0x5a_cpol0_cpha0_trigger_cs_rising_csactivehigh_ok.sr", "params": "6 5 3 high 0 0 msb CS# CLK MOSI active-high 0 0 msb-first"})
spi_file.append({"file": "data/spi/spi_0x35_cpol0_cpha1_trigger_cs_falling_ok.sr", "params": "6 5 3 low 0 1 msb CS# CLK MOSI active-low 0 1 msb-first"})
spi_file.append({"file": "data/spi/spi_0x35_cpol1_cpha0_trigger_cs_falling_ok.sr", "params": "6 5 3 low 1 0 msb CS# CLK MOSI active-low 1 0 msb-first"})
spi_file.append({"file": "data/spi/spi_0x35_cpol1_cpha0_trigger_clk_rising_ok.sr", "params": "6 5 3 low 1 0 msb CS# CLK MOSI active-low 1 0 msb-first"})
spi_file.append({"file": "data/spi/spi_0x35_cpol1_cpha1_trigger_clk_falling_ok.sr", "params": "6 5 3 low 1 1 msb CS# CLK MOSI active-low 1 1 msb-first"})
spi_file.append({"file": "data/spi/spi_0x35_cpol1_cpha1_trigger_clk_rising_ok.sr", "params": "6 5 3 low 1 1 msb CS# CLK MOSI active-low 1 1 msb-first"})
spi_file.append({"file": "data/spi/spi_0x35_cpol1_cpha1_trigger_cs_falling_ok.sr", "params": "6 5 3 low 1 1 msb CS# CLK MOSI active-low 1 1 msb-first"})
spi_file.append({"file": "data/spi/spi_0x5a_cpol1_cpha1_trigger_clk_rising_ok.sr", "params": "6 5 3 low 1 1 msb CS# CLK MOSI active-low 1 1 msb-first"})
spi_file.append({"file": "data/spi/spi_0x5a_cpol1_cpha1_trigger_none_ok.sr", "params": "6 5 3 low 1 1 msb CS# CLK MOSI active-low 1 1 msb-first"})
spi_file.append({"file": "data/spi/spi_0x35_cpol0_cpha0_trigger_clk_falling_ok.sr", "params": "6 5 3 low 0 0 msb CS# CLK MOSI active-low 0 0 msb-first"})
spi_file.append({"file": "data/spi/spi_0x5a_cpol1_cpha1_trigger_none_csactivehigh_ok.sr", "params": "6 5 3 high 1 1 msb CS# CLK MOSI active-high 1 1 msb-first"})
spi_file.append({"file": "data/spi/spi_0x5a_cpol1_cpha1_trigger_cs_falling_ok.sr", "params": "6 5 3 low 1 1 msb CS# CLK MOSI active-low 1 1 msb-first"})


# Different
i2c_file.append({"file": "data/i2c/rtc_ds1307_200khz.sr", "params": "1 2"})
i2c_file.append({"file": "data/i2c/a2_dummy_write.sr", "params": "1 2"})
i2c_file.append({"file": "data/i2c/rtc_epson_8564je.sr", "params": "1 2"})
i2c_file.append({"file": "data/i2c/rtc_epson_8564je_snippet.sr", "params": "1 2"})
i2c_file.append({"file": "data/i2c/xfp.sr", "params": "1 2"})
i2c_file.append({"file": "data/i2c/samsung_le46b620r3p.sr", "params": "1 2"})
i2c_file.append({"file": "data/i2c/samsung_syncmaster203b.sr", "params": "1 2"})
i2c_file.append({"file": "data/i2c/samsung_syncmaster245b.sr", "params": "1 2"})
i2c_file.append({"file": "data/i2c/lcsoft-mini-board-fx2-init.sr", "params": "1 2"})
i2c_file.append({"file": "data/i2c/microsoft-wireless-optical-mouse-init.sr", "params": "1 2"})
i2c_file.append({"file": "data/i2c/24aa025uid_bytewrite5_6ms_delay.sr", "params": "1 2"})
i2c_file.append({"file": "data/i2c/24aa025uid_bytewrite5_6ms_delay_trigger_sda_low.sr", "params": "1 2"})
i2c_file.append({"file": "data/i2c/24aa025uid_bytewrite8_6ms_delay.sr", "params": "1 2"})
i2c_file.append({"file": "data/i2c/24aa025uid_bytewrite8_6ms_delay_trigger_sda_low.sr", "params": "1 2"})
