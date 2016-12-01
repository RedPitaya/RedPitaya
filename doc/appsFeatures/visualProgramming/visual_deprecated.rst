~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Connectors to Red Pitaya motherboard
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

While it is possible to connect sensors and indicators directly to Red Pitaya *E1* and *E2* connectors,
it is recommended to use the extension module.
It enables the user to connect multiple peripherals without improvised wiring.
It is also possible for developers to create their own custom extensions.

""""""""""""
Connector E1
""""""""""""

Connector E1 provides 16 *GPIO* (general purpose input/output) signals.
All GPIO can be used to read sensor data, or drive indicators.

+----------+--------+--------+----------+
| function |    pin |  pin   | function |
+==========+========+========+==========+
| GND      | ``26`` | ``25`` | GND      |
+----------+--------+--------+----------+
| NC       | ``24`` | ``23`` | NC       |
+----------+--------+--------+----------+
| NC       | ``22`` | ``21`` | NC       |
+----------+--------+--------+----------+
| NC       | ``20`` | ``19`` | NC       |
+----------+--------+--------+----------+
| D[15]    | ``18`` | ``17`` | D[7]     |
+----------+--------+--------+----------+
| D[14]    | ``16`` | ``15`` | D[6]     |
+----------+--------+--------+----------+
| D[13]    | ``14`` | ``13`` | D[5]     |
+----------+--------+--------+----------+
| D[12]    | ``12`` | ``11`` | D[4]     |
+----------+--------+--------+----------+
| D[11]    | ``10`` |  ``9`` | D[3]     |
+----------+--------+--------+----------+
| D[10]    |  ``8`` |  ``7`` | D[2]     |
+----------+--------+--------+----------+
| D[ 9]    |  ``6`` |  ``5`` | D[1]     |
+----------+--------+--------+----------+
| D[ 8]    |  ``4`` |  ``3`` | D[0]     |
+----------+--------+--------+----------+
| +3.3V    |  ``2`` |  ``1`` | +3.3V    |
+----------+--------+--------+----------+

""""""""""""
connector E2
""""""""""""

Connector E2 provides 4 *AI* (analog input) signals, 4 *AO* (analog output) signals
and signals for serial protocols *UART*, *SPI* and *I2C*.
Analog inputs can be used to connect analog sensors.
Analog outputs can be used to drive some indicators.
Serial protocols can be used to connect more complex sensors, indicators and other peripherals.

+----------+--------+--------+----------+
| function |    pin |  pin   | function |
+==========+========+========+==========+
| GND      | ``26`` | ``25`` | GND      |
+----------+--------+--------+----------+
| ADC_CLK- | ``24`` | ``23`` | ADC_CLK+ |
+----------+--------+--------+----------+
| GND      | ``22`` | ``21`` | GND      |
+----------+--------+--------+----------+
| AO[3]    | ``20`` | ``19`` | AO[2]    |
+----------+--------+--------+----------+
| AO[1]    | ``18`` | ``17`` | AO[0]    |
+----------+--------+--------+----------+
| AI[3]    | ``16`` | ``15`` | AI[2]    |
+----------+--------+--------+----------+
| AI[1]    | ``14`` | ``13`` | AI[0]    |
+----------+--------+--------+----------+
| I2C_GND  | ``12`` | ``11`` | common   |
+----------+--------+--------+----------+
| I2C SDA  | ``10`` |  ``9`` | I2C_SCK  |
+----------+--------+--------+----------+
| UART_RX  |  ``8`` |  ``7`` | UART_TX  |
+----------+--------+--------+----------+
| SPI_CS   |  ``6`` |  ``5`` | SPI_CLK  |
+----------+--------+--------+----------+
| SPI_MISO |  ``4`` |  ``3`` | SPI_MOSI |
+----------+--------+--------+----------+
| -4V      |  ``2`` |  ``1`` | +5V      |
+----------+--------+--------+----------+
