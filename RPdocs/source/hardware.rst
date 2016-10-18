Hardware
========

.. zumret

..                  STEM boards
..                                   STEM 125-10

The central component of the Red Pitaya board is a Xilinx ZC7Z010 system-on-chip (SoC) device.
It contains a dual-core ARM9 processor clocked at 800 MHz, which hosts a Linux distribution on the Red Pitaya board.
The SoC also contains an FPGA with 28k logic cells, a 2.1 Mb of block RAM (BRAM), and 80 DSP slices. The SoC is
equipped with additional 512MB of RAM memory.
The board features an 1Gbit Ethernet port, a USB Host port and a USB Com port for easy communication with a PC over a 
serial interface.
Operating system is loaded from an SD card. Xilinx ZC7Z010 has integrated general purpose digital I/O, I2C,UART,SPI 
interfaces and additional
relatively slow(100kS/s) A/D and D/A converters which can be accessed through extension connectors E1 and E2. 
The board features a powerful analog frontend for analog signal acquisition and generation. Main components of analog 
frontend are are two 14-bit ADC and two 14-bit DAC with a maximum sample rate of 125 MS/s.    

.. slika http://wiki.redpitaya.com/index.php?title=Hardware_Overview ( Figure: Red Pitaya board HW overview )

Fast analog inputs and outputs are one of the main features of Red Pitaya board since they are used when running web 
applications such is Oscilloscope 
or implementing custom signal processing. Extension connectors can be used for variety of purposes since they have 16 
digital inputs/outputs directly connected to the FPGA.
Integrated SoC’s slow analog inputs/outputs can be used for slow data logging, sensor measurement (temperature...). 
Extension connectors are also used for attaching extension modules for expanding Red Pitaya T&M capabilities.
Such extension module is for LCR meter application or Sensor extension module which enables simple connections of 
different sensors.

=================== ============================================================
Name                Description
=================== ============================================================
Processor           Dual core ARM Cortex A9+ FPGA FPGA Xilinx Zynq 7010 SoC
RAM                 DDR3 RAM 512MB (4Gb) System memory
System memory       System memory microSD up to 32Gb
Network connection  1000Base-T Ethernet connection
USB                 USB 2.0
Console connection  micro USB
Power connector     micro USB - 5V, 2A max
=================== ============================================================

.. slike http://wiki.redpitaya.com/index.php?title=Hardware_Overview

Analog inputs
-------------

    - Number of channels: 2
    - Bandwidth: 50 MHz (3 dB)
    - Sample rate: 125 Msps 
    - ADC resolution 14 bits 
    - Input coupling: DC 
    - Input noise level: < -­119 dBm /Hz (D)
    - Input impedance: 1 MΩ // 10 pF (A,B) 
    - Full scale voltage: 2Vpp, (46 Vpp for low­gain jumper setting) (T,V) 
    - DC offset error: <5 % FS (G) 
    - gain error: < 3% (at high gain jumper setting), <10% (at low gain jumper setting) (G) 
    - Absolute maximum input voltage rating: 30 V (S) (1500 V ESD) 
    - Overload protection: protection diodes (under the input voltage rating conditions) 
    - Input channel isolation: typical performance 65 dB @ 10 kHz, 50 dB @ 100 kHz, 55 dB @ 1 M, 55 dB @ 10 MHz, 52 dB 
      @ 20 MHz, 48 dB @ 30 MHz, 44 dB @ 40 MHz, 40 dB @ 50 MHz. (C) 
    - Harmonics 
    - at -­3 dBFS: typical performance <­-45 dBc (E) 
    - at ­-20 dBFS: typical performance <­-60 dBc (E) 
    - Spurious frequency components: Typically <­-90 dBFS (F) 
    - Connector type: SMA (U) 
    - Frequency response is adjusted by digital compensation 

Analog outputs
--------------

    - Number of channels: 2 
    - Bandwidth: 50 MHz (3 dB) (K) 
    - Sample rate: 125 Msps 
    - DAC resolution: 14 bits 
    - Output coupling: DC 
    - Load impedance: 50 Ω (J) 
    - Output slew rate limit: 200 V/us 
    - Connector type: SMA (U) 
    - DC offset error: < 5% FS (G) 
    - Gain error: < 5% (G) 
    - Full scale power: > 9 dBm (L) 
    - Harmonics: typical performance: (at ­8 dBm) 
    - ­ -51 dBc @ 1 MHz 
    - ­ -49 dBc @ 10 MHz 
    - ­ -48 dBc @ 20 MHz 
    - ­ -53 dBc @ 45 MHz 

Auxiliary analog input channels
-------------------------------
    
    - Number of channels: 4 
    - Nominal sampling rate: 100 ksps (H) 
    - ADC resolution 12 bits 
    - Connector: dedicated pins on IDC connector E2 (pins 13,14,15,16) 
    - Input voltage range: 0 to +3.5 V 
    - Input coupling: DC 

Auxiliary analog output channels 
--------------------------------

    - Number of channels: 4 
    - Output type: Low pass filtered PWM (I) 
    - PWM time resolution: 4ns (1/250 MHz)
    - Connector: dedicated pins on IDC connector E2 (pins 17,18,19,20) v - Output voltage range: 0 to +1.8 V 
    - Output coupling: DC 

General purpose digital input/output channels: (N) 
--------------------------------------------------

    - Number of digital input/output pins: 16 
    - Voltage level: 3.3 V 
    - Direction: configurable 
    - Location: IDC connector E1 (pins 3­24 ) 

Extension connector 
-------------------

    - Connector: 2 x 26 pins IDC (M) 
    - Power supply: 
    - Available voltages: +5V, +3.3V, ­3.3V 
    - Current limitations: 500 mA for +5V and +3.3V (to be shared between extension module and USB devices), 50 mA 
      for ­3.3V supply. 
    
Extension connector E1
^^^^^^^^^^^^^^^^^^^^^^

- 3v3 power source
- 16 single ended or 8 differential digital I/Os with 3,3V logic levels

===  =========== =============== ======================== ==============
Pin  Description FPGA pin number FPGA pin description     Voltage levels
===  =========== =============== ======================== ==============
1    3V3  
2    3V3
3    DIO0_P      G17             IO_L16P_T2_35 (EXT TRIG) 3.3V
4    DIO0_N      G18             IO_L16N_T2_35            3.3V
5    DIO1_P      H16             IO_L13P_T2_MRCC_35       3.3V
6    DIO1_N      H17             IO_L13N_T2_MRCC_35       3.3V
7    DIO2_P      J18             IO_L14P_T2_AD4P_SRCC_35  3.3V
8    DIO2_N      H18             IO_L14N_T2_AD4N_SRCC_35  3.3V
9    DIO3_P      K17             IO_L12P_T1_MRCC_35       3.3V
10   DIO3_N      K18             IO_L12N_T1_MRCC_35       3.3V
11   DIO4_P      L14             IO_L22P_T3_AD7P_35       3.3V
12   DIO4_N      L15             IO_L22N_T3_AD7N_35       3.3V
13   DIO5_P      L16             IO_L11P_T1_SRCC_35       3.3V
14   DIO5_N      L17             IO_L11N_T1_SRCC_35       3.3V
15   DIO6_P      K16             IO_L24P_T3_AD15P_35      3.3V
16   DIO6_N      J16             IO_L24N_T3_AD15N_35      3.3V
17   DIO7_P      M14             IO_L23P_T3_35            3.3V
18   DIO7_N      M15             IO_L23N_T3_35            3.3V
19   NC
20   NC
21   NC
22   NC
23   NC
24   NC
25   GND
26   GND
===  =========== =============== ======================== ==============

All DIOx_y pins are LVCMOS33. abs. max. ratings are: min. –0.40V max. 3.3V + 0.55V    

Extension connector E2
^^^^^^^^^^^^^^^^^^^^^^

    - +5V & -3V3 power source
    - SPI, UART, I2C
    - 4 x slow ADCs
    - 4 x slow DACs
    - Ext. clock for fast ADC
 
.. Table 6: Extension connector E2 pin description

===  ====================== =============== ==================== ==============
Pin  Description            FPGA pin number FPGA pin description Voltage levels
===  ====================== =============== ==================== ==============
1    +5V                                                                         
2    -3.4V (50mA)\ :sup:`1`                                                                         
3    SPI(MOSI)              E9              PS_MIO10_500         3.3V
4    SPI(MISO)              C6              PS_MIO11_500         3.3V
5    SPI(SCK)               D9              PS_MIO12_500         3.3V
6    SPI(CS#)               E8              PS_MIO13_500         3.3V
7    UART(TX)               C8              PS_MIO08             3.3V
8    UART(RX)               C5              PS_MIO09             3.3V
9    I2C(SCL)               B9              PS_MIO50_501         3.3V
10   I2C(SDA)               B13             PS_MIO51_501         3.3V
11   Ext com.mode                                                GND (default)
12   GND                                                       
13   Analog Input 0                                              0-3.5V
14   Analog Input 1                                              0-3.5V
15   Analog Input 2                                              0-3.5V
16   Analog Input 3                                              0-3.5V
17   Analog Output 0                                             0-1.8V
18   Analog Output 1                                             0-1.8V
19   Analog Output 2                                             0-1.8V
20   Analog Output 3                                             0-1.8V
21   GND                                                       
22   GND                                                       
23   Ext Adc CLK+                                                LVDS
24   Ext Adc CLK-                                                LVDS
25   GND                                                       
26   GND                                                       
===  ====================== =============== ==================== ==============

\ :sup:`1` Red Pitaya Version 1.0 has -3.3V on pin 2. Red Pitaya Version 1.1 has -3.4V on pin 2.
Schematics of extension connectors is shown in picture bellow.

.. slika http://wiki.redpitaya.com/index.php?title=Extension_connectors

**Notes:**

#. Input capacitance depends on jumper settings and may vary. 
#. A 50 Ω termination can be connected through an SMA tee in parallel to the input for measurements in a 50 Ω system. 
#. Crosstalk measured with high gain jumper setting on both channels. The SMA connectors not involved in the
   measurement are terminated.
#. Measurement referred to high gain jumper setting, with limited environmental noise, inputs and outputs terminated,
   output signals disabled, PCB grounded through SMA ground. The specified noise floor measurement is calculated from 
   the standard deviation of 16k contiguous samples at full rate. (Typically full bandwidth std(Vn) < 2 mV). Noise 
   floor specification does not treat separately spurious spectral components and represents time domain noise average 
   referred to a 1 Hz bandwidth. In presence of spurious components the actual noise floor would result lower.
#. Measurement referred at high gain jumper setting, inputs matched and outputs terminated, outputs signal disabled, 
   PCB grounded through SMA ground. 
#. Measurement referred to high gain jumper setting, inputs and outputs terminated, outputs signal disabled, PCB 
   grounded through SMA ground. 
#. Further corrections can be applied through more precise gain and DC offset calibration. 
#. Default software enables sampling at CPU dependent speed. The acquisition of sequence at 100 ksps rate requires the
   implementation of additional FPGA processing.
#. First order low pass filter implementation. Additional filtering can be externally applied according to application 
   requirements. 
#. The output channels are designed to drive 50 Ω loads. Terminate outputs when channels are not used. Connect 
   parallel 50 Ω load (SMA tee junction) in high impedance load applications. 
#. Measured at ­10 dBm output power level 
#. Typical power level with 1 MHz sine is 9.5 dBm. Output power is subject to slew rate limitations. 
#. Detailed scheme available within documentation (Red_Pitaya_Schematics_v1.0.1.pdf) 
#. To avoid speed limitations on digital General Purpose Input / Output pins are directly connected to FPGA. FPGA
   decoupling and pin protection is to be addressed within extension module designs. User is responsible for pin 
   handling. 
#. The use of not approved power supply may deteriorate performance or damage the product. 
#. Heatsink must be installed and board must be operated on a flat surface without airflow obstructions. Operation at 
   higher ambient temperatures, lower pressure conditions or within enclosures to be addressed by means of adequate 
   ventilation. The operation of the product is automatically disabled at increased temperatures. 
#. Some parts may become hot during and after operation. Do not touch them. 
#. Measurement performance is specified within this range. 
#. Valid for low frequency signals. For input signals that contain frequency components beyond 1 kHz, the full scale
   value defines the maximum admissible input voltage.
#. Jumper settings are limited to the positions described in the user manual. Any other configuration or use of
   different jumper type may damage the product. 
#. SMA connectors on the cables connected to Red Pitaya must correspond to the standard MIL­C­39012. It’s Important that
   central pin is of suitable length, otherwise the SMA connector installed in Red Pitaya will mechanically damage the
   SMA connector. Central pin of the SMA connector on Red Pitaya will loose contact to the board and the board will 
   not be possible to repair due to the mechanical damage (separation of the pad from the board). 
#. Jumpers are not symmetrical, they have latches. Always install jumpers with the latch on its outer side in order to
   avoid problems with hard to remove jumpers. 
#. Dimensions are rounded to the nearest millimeter. For exact dimensions, please see the Technical drawings and 
   product model. (Red_Pitaya_Dimensions_v1.0.1.pdf) 

Information furnished by Red Pitaya d.d. is believed to be accurate and reliable. However, no responsibility is 
assumed for its use. Contents may be subject to change without any notice. 

Extension module template
-------------------------

.. TODO

External ADC clock
------------------

.. slik http://wiki.redpitaya.com/index.php?title=External_ADC_clock

ADC clock can be provided by:
    * On board 125MHz XO (default)
    * From external source / through extension connector E2 (R25,R26 should be moved to location R23,R24)
    * Directly from FPGA (R25,R26 should be moved to location R27,R28)

.. Schematic:

STEM schematics
---------------

Red Pitaya board HW FULL schematics are not available. Red Pitaya has an open source code but not an open hardware 
schematics. Nonetheless, DEVELOPMENT schematics are available 
`here <https://dl.dropboxusercontent.com/s/jkdy0p05a2vfcba/Red_Pitaya_Schematics_v1.0.1.pdf>`_ .

This schematic will give you information about HW configuration, FPGA pin connection and similar.

Analog inptus & outputs calibration
-----------------------------------

.. http://wiki.redpitaya.com/index.php?title=Analog_Inputs_and_Outputs_calibration
.. TODO: New Oscilloscope&Signal bad link

Calibration processes can be performed using New Oscilloscope&Signal generator app. or using calib command line 
utility. When performing calibration with the new Oscilloscope&Signal generator application just select 
Settings->Calibration and follow instructions.

    - Calibration using calib utility
    
Start your Red Pitaya and connect to it via `Terminal <http://redpitaya.com/faq-page/#>`_.

.. code-block:: shell-session
   
    redpitaya> calib
 
    Usage: calib [OPTION]...
    
    OPTIONS:
     -r    Read calibration values from eeprom (to stdout).
     -w    Write calibration values to eeprom (from stdin).
     -f    Use factory address space.
     -d    Reset calibration values in eeprom with factory defaults.
     -v    Produce verbose output.
     -h    Print this info.

The EEPROM is a non-volatile memory, therefore the calibration coefficients will not change during Red Pitaya power 
cycles, nor will they change with software upgrades via Bazaar or with manual modifications of the SD card content. 
Example of calibration parameters readout from EEPROM with verbose output::

    redpitaya> calib -r -v
    FE_CH1_FS_G_HI = 45870551      # IN1 gain coefficient for LV (+/- 1V range)  jumper configuration.
    FE_CH2_FS_G_HI = 45870551      # IN2 gain coefficient for LV (+/- 1V range)  jumper configuration.
    FE_CH1_FS_G_LO = 1016267064    # IN1 gain coefficient for HV (+/- 20V range) jumper configuration.
    FE_CH2_FS_G_LO = 1016267064    # IN2 gain coefficient for HV (+/- 20V range) jumper configuration.
    FE_CH1_DC_offs = 78            # IN1 DC offset  in ADC samples.
    FE_CH2_DC_offs = 25            # IN2 DC offset  in ADC samples.
    BE_CH1_FS = 42755331           # OUT1 gain coefficient.
    BE_CH2_FS = 42755331           # OUT2 gain coefficient.
    BE_CH1_DC_offs = -150          # OUT1 DC offset in DAC samples.
    BE_CH2_DC_offs = -150          # OUT2 DC offset in DAC samples.

Example of the same calibration parameters readout from EEPROM with non-verbose output, suitable for editing within 
scripts::

    redpitaya> calib -r
           45870551            45870551          1016267064          1016267064 

You can write changed calibration parameters using **calib -w** command:
1. Type calib -w in to command line (terminal)
2. Press enter
3. Paste or write new calibration parameters
4. Press enter

.. code-block:: shell-session
   
    redpitaya> calib -w
        40000000           45870551          1016267064          1016267064 

Should you bring the calibration vector to an undesired state, you can always reset it to factory defaults using::

    redpitaya> calib -d

DC offset calibration parameter can be obtained as average of acquired signal at grounded input. Gains parameter can 
be calculated by using reference voltage source and old version of an Oscilloscope application. Start Oscilloscope
app. connect ref. voltage to the desired input and take measurements. Change gain calibration parameter using 
instructions above, reload the Oscilloscope application and make measurements again with new calibration parameters. 
Gain parameters can be optimized by repeating calibration and measurement step. 

In the table bellow typical results after calibration are shown. 

**INPUTS**

=========================== =============== ===========
Parameter                   Jumper settings Value
=========================== =============== ===========
DC GAIN ACCURACY @ 122 kS/s LV              0.2%
DC OFFSET @ 122 kS/s        LV              +/- 0.5 mV
DC GAIN ACCURACY @ 122 kS/s HV              0.5%
DC OFFSET @ 122 kS/s        HV              +/- 5 mV
=========================== =============== ===========

AC gain accuracy can be extracted form Frequency response - Bandwidth given in Figure: 
`Fast Analog Inputs Bandwidth <http://wiki.redpitaya.com/index.php?title=File:Bandwidth_of_Fast_Analog_Inputs.png>`_.

**OUTPUTS**

Calibration is performed in noise controlled environment. Inputs and outputs gains are calibrated with 0.02% and
0.003% DC reference voltage standards. Input gains calibration is performed in medium size timebase range. Red Pitaya
is non-shielded device and its inputs/outputs ground is not connected to the earth grounding as it is in case of 
classical Oscilloscopes. To achieve calibration results given below, Red Pitaya must be grounded and shielded.

.. Table: Typical specification after calibration

================= ==========
Parameter         Value
================= ==========
DC GAIN ACCURACY  0.4%
DC OFFSET         +/- 4 mV
RIPPLE(@ 0.5V DC) 0.4 mVpp
================= ==========

AC gain accuracy can be extracted form 
`Frequency response <http://wiki.redpitaya.com/index.php?title=File:Fast_Analog_Outputs_Bandwidt.png>`_.

Mechanical specifications (STEP model)
--------------------------------------

`3D STEP model v1.1.1 <https://www.dropbox.com/s/skbmydtjslradwx/Red_Pitaya_3Dmodel_v1.1.1.zip>`_
`3D STEP model v1.0.1 <https://www.dropbox.com/s/s6d65stm6qz5hdp/Red_Pitaya_3Dmodel_v1.0.1.zip>`_

Certificates
------------

Besides the functional testing Red Pitaya passed the safety and electromagnetic compatibility (EMC) tests at an 
external `testing and certification institute <http://www.siq.si/?L=3>`_.

.. -TODO  wiki (http://wiki.redpitaya.com/index.php?title=Certificates_%26_test_reports)

Cooling options 
---------------

.. TODO http://forum.redpitaya.com/viewtopic.php?f=9&t=380
    imamo kaj slik se ostalih hladilnih sistemov?

Powering Red Pitaya through extension connector
-----------------------------------------------

Red Pitaya can be also powered through pin1 of the extension connector E2, but in such case external protection must
be provided by the user in order to protect the board!

.. TODO Protection.png

Protection circuit between +5V that is provided over micro USB power connector and +5VD that is connected to pin1 of 
the extension connector E2.

.. LEDs function and description
.. -----------------------------

.. TODO

Extension modules
-----------------

Impedance analyzer (LCR)
^^^^^^^^^^^^^^^^^^^^^^^^

Impedance analyzer application enables measurements of Impedance, Phase and other parameters of selected DUT (Device 
Under Test). Measurements can be performed in “Frequency sweep” mode with 1Hz of frequency resolution or in 
“Measurements sweep” mode with desired numbers of measurement at constant frequency. Selectable frequency range is
from 1Hz to 60MHz, although the recommended frequency range is up to 1MHz*. Impedance range is from 0.1 Ohm – 10 
MOhm*. When using Impedance analyzer application with LCR Extension module insert 0 in the shunt resistor field.

**Note:** Impedance range is dependent on the selected frequency and maximum accuracy and suitable measurement can not
be performed at all frequencies and impedance ranges. Impedance range is given in picture bellow. Range for Capacitors
or Inductors can be extrapolated from given picture. Basic accuracy of the Impedance analyzer is 5%. Impedance 
analyzer application is calibrated for 1 m Kelvin probes. More accurate measurements can be performed in Measurement 
sweep at constant frequency.

.. TODO LCR range.png

When using Impedance analyzer application optimal results are achieved when the Red Pitaya GND is connected to your 
mains EARTH lead as is shown below. We also recommend shielding of Red Pitaya and LCR extension module.

.. TODO E module connection.png

On pictures below are shown comparison measurements of the selected DUT. Measurements are taken with Red Pitaya and 
Keysight precision LCR meter. From this plots you can extract basic Red Pitaya accuracy. Notice Red Pitaya LCR 
meter / Impedance analyzer are not certificated for certain accuracy or range.

.. TODO LCR 100R.png LCR 100K.png LCR 1M.png

Impedance analyzer application can be used without LCR Extension module using manual setting of shunt resistor. This 
option is described below. Notice that you will need to change “C_cable” parameter in the code when using your setup.

.. TODO Impedance analyzer manaul R Shunt.png

.. TODO Sensor module
.. TODO ^^^^^^^^^^^^^

.. TODO LA ext. module
.. TODO ^^^^^^^^^^^^^^

.. TODO Casings
.. TODO -------

.. TODO Alu
.. TODO ^^^

.. TODO Acrylic
.. TODO ^^^^^^^
