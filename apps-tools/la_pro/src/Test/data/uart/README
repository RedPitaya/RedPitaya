-------------------------------------------------------------------------------
Hello World UART output using various options
-------------------------------------------------------------------------------

This is a collection of example UART communication (only in one direction).
The repeatedly transmitted data is "Hello World!\r\n", using various settings,
such as 8n1, 8o1, 8e1, 7o1, 7e1, etc.

The hardware sending the data is an Olimex STM32-H103 eval kit with an
ST STM32 (ARM Cortex-M3) microcontroller. The firmware used is based on a
simple libopencm3 UART example. The USART1 (USART1.TX pin) is used.

For details see:
http://libopencm3.org
http://libopencm3.git.sourceforge.net/git/gitweb.cgi?p=libopencm3/libopencm3;a=tree;f=examples/stm32/f1/stm32-h103/usart_printf
http://olimex.com/dev/stm32-h103.html


Logic analyzer setup
--------------------

The logic analyzer used was a ChronoVu LA8 at a sample rate of 5MHz (for baud
rates 921600 - 230400), 1MHz (115200 - 19200), and 625kHz (9600 - 1200):

  Probe       UART
  ----------------
  0 (green)   TX


Data
----

The sigrok command line used was:

  sigrok-cli -d 0:samplerate=<hz> --samples 8388608 -p '1=TX' -o <file>.sr

We capture as many samples as fit into the 8MByte buffer of the logic analyzer.

