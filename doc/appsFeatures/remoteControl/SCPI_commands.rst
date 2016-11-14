*******************************
List of supported SCPI commands 
*******************************

.. (link - https://dl.dropboxusercontent.com/s/eiihbzicmucjtlz/SCPI_commands_beta_release.pdf)

Table of correlated SCPI and API commands on Red Pitaya.

==============
LEDs and GPIOs
==============

Parameter options:

* ``<dir> = {OUTP,INP}``
* ``<gpio> = {{DIO0_P...DIO7_P}, {DIO0_N...DIO7_N}}``
* ``<led> = {LED0...LED8}``
* ``<pin> = {gpio, led}``
* ``<state> = {0,1}``

.. tabularcolumns:: |p{28mm}|p{28mm}|p{28mm}|

+------------------------------------+-------------------------+------------------------------------------------------+
| SCPI                               | API                     | description                                          |
+------------------------------------+-------------------------+------------------------------------------------------+
| | ``DIG:PIN:DIR <dir>,<gpio>``     | ``rp_DpinSetDirection`` | Set direction of digital pins to output or input.    |
| | Examples:                        |                         |                                                      |                       
| | ``DIG:PIN:DIR OUTP,DIO0_N``      |                         |                                                      |  
| | ``DIG:PIN:DIR INP,DIO1_P``       |                         |                                                      |                  
+------------------------------------+-------------------------+------------------------------------------------------+
| | ``DIG:PIN <pin>,<state>``        | ``rp_DpinSetState``     | Set state of digital outputs to 1 (HIGH) or 0 (LOW). |
| | Examples:                        |                         |                                                      |
| | ``DIG:PIN DIO0_N,1``             |                         |                                                      |
| | ``DIG:PIN LED2,1``               |                         |                                                      |
+------------------------------------+-------------------------+------------------------------------------------------+
| | ``DIG:PIN? <pin>`` > ``<state>`` | ``rp_DpinGetState``     | Get state of digital inputs and outputs.             |
| | Examples:                        |                         |                                                      |
| | ``DIG:PIN? DIO0_N``              |                         |                                                      |
| | ``DIG:PIN? LED2``                |                         |                                                      |
+------------------------------------+-------------------------+------------------------------------------------------+

=========================
Analog Inputs and Outputs
=========================

Parameter options:

* ``<ain> = {AIN0, AIN1, AIN2, AIN3}``
* ``<aout> = {AOUT0, AOUT1, AOUT2, AOUT3}``
* ``<pin> = {ain, aout}``
* ``<value> = {value in Volts}``
   
.. tabularcolumns:: |p{28mm}|p{28mm}|p{28mm}|p{28mm}|

+---------------------------------------+---------------------+------------------------------------------------------+
| SCPI                                  | API                 | description                                          |
+---------------------------------------+---------------------+------------------------------------------------------+
| | ``ANALOG:PIN <pin>,<value>``        | ``rp_ApinSetValue`` | | Set analog voltage on slow analog outputs.         |
| | Examples:                           |                     | | Voltage range of slow analog outputs is: 0 - 1.8 V |
| | ANALOG:PIN AOUT0,1                  |                     |                                                      |
| | ANALOG:PIN AOUT2,1.34               |                     |                                                      |
+---------------------------------------+---------------------+------------------------------------------------------+
| | ``ANALOG:PIN? <pin>`` > ``<value>`` | ``rp_ApinGetValue`` | | Read analog voltage from slow analog inputs.       |
| | Examples:                           |                     | | Voltage range of slow analog inputs is: 0 3.3 V    |
| | ANALOG:PIN? AOUT0                   |                     |                                                      |
| | ANALOG:PIN? AIN2                    |                     |                                                      |
+---------------------------------------+---------------------+------------------------------------------------------+

================
Signal Generator
================

Parameter options:

* ``<n> = {1,2}`` (set channel OUT1 or OUT2)
* ``<state> = {ON,OFF}`` Default: ``OFF``
* ``<frequency> = {0Hz...62.5e6Hz}`` Default: ``1000``
* ``<func> = {SINE, SQUARE, TRIANGLE, SAWU, SAWD, PWM, ARBITRARY}`` Default: ``SINE``
* ``<amplitude> = {-1V...1V}`` Default: ``1``
* ``<offset> = {-1V...1V}`` Default: ``0``
* ``<phase> = {-360deg ... 360deg}`` Default: ``0``
* ``<dcyc> = {0%...100%}`` Default: ``50``
* ``<array> = {value1, ...}`` max. 16k values, floats in the range -1 to 1
* ``<burst> = {ON,OFF}`` Default: ``OFF``
* ``<count> = {1...50000, INF}`` ``INF`` = infinity/continuous, Default: ``1``
* ``<time> = {1us-500s}`` Value in *us*.
* ``<trigger> = {EXT_PE, EXT_NE, INT, GATED}``
   * ``EXT`` = External
   * ``INT`` = Internal
   * ``GATED`` = gated busts

.. tabularcolumns:: |p{28mm}|p{28mm}|p{28mm}|

+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| SCPI                                 | API                        | description                                                              |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``OUTPUT<n>:STATE <state>``        | | ``rp_GenOutEnable``      | Disable or enable fast analog outputs.                                   |
| | Examples:                          | | ``rp_GenOutDisable``     |                                                                          |
| | OUTPUT1:STATE ON                   |                            |                                                                          |
| | OUTPUT2:STATE OFF                  |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR<n>:FREQ:FIX <frequency>``   | ``rp_GenFreq``             | Set frequency of fast analog outputs.                                    |
| | Examples:                          |                            |                                                                          |
| | SOUR1:FREQ:FIX 1000                |                            |                                                                          |
| | SOUR2:FREQ:FIX 100000              |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR<n>:FUNC <func>``            | ``rp_GenWaveform``         | Set waveform of fast analog outputs.                                     |
| | Examples:                          |                            |                                                                          |
| | SOUR1:FUNC SINE                    |                            |                                                                          |
| | SOUR2:FUNC TRIANGLE                |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR<n>:VOLT <amplitude>``       | ``rp_GenAmp``              | | Set amplitude voltage of fast analog outputs.                          |
| | Examples:                          |                            | | Amplitude + offset value must be less than maximum output range +/- 1V |
| | SOUR1:VOLT 1                       |                            |                                                                          |
| | SOUR2:VOLT 0.5                     |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR<n>:VOLT:OFFS <offset>``     | ``rp_GenOffset``           | | Set offset voltage of fast analog outputs.                             |
| | Examples:                          |                            | | Amplitude + offset value must be less than maximum output range +/- 1V |
| | SOUR1:VOLT:OFFS 0.2                |                            |                                                                          |
| | SOUR1:VOLT:OFFS 0.1                |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR<n>:PHAS <phase>``           | ``rp_GenPhase``            | Set phase of fast analog outputs.                                        |
| | Examples:                          |                            |                                                                          |
| | SOUR2:PHAS 30                      |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR<n>:DCYC <par>``             | ``rp_GenDutyCycle``        | Set duty cycle of PWM waveform.                                          |
| | Examples:                          |                            |                                                                          |
| | SOUR1:DCYC 34                      |                            |                                                                          |
| | SOUR2:DCYC 50                      |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR<n>:TRAC:DATA:DATA <array>`` | ``rp_GenArbWaveform``      | Import data for arbitrary waveform generation.                           |
| | Examples:                          |                            |                                                                          |
| | SOUR1:TRAC:DATA:DATA               |                            |                                                                          |
| | 1,0.5,0.2                          |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR<n>:BURS:STAT <burst>``      | ``rp_GenMode``             | Enable or disable burst (pulse) mode.                                    |
| | Examples:                          |                            | Red Pitaya will generate **R** number of **N** periods of signal         |
| | SOUR1:BURS:STAT ON                 |                            | and then stop. Time between bursts is **P**.                             |
| | SOUR1:BURS:STAT OFF                |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR<n>:BURS:NCYC <count>``      | ``rp_GenBurstCount``       | Set N number of periods in one burst.                                    |
| | Examples:                          |                            |                                                                          |
| | SOUR1:BURS:NCYC 3                  |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR1:BURS:NOR <count>``         | ``rp_GenBurstRepetitions`` | Set R number of repeated bursts.                                         |
| | Examples:                          |                            |                                                                          |
| | SOUR1:BURS:NOR 5                   |                            |                                                                          |
+--------------------------------------+----------------------------+---------------------------+----------------------------------------------+
| | ``SOUR1:BURS:INT:PER <time>``      | ``rp_GenBurstPeriod``      | Set P total time of one burst in in micro seconds.                       |
| | Examples:                          |                            | This includes the signal and delay.                                      |
| | SOUR1:BURS:INT:PER 1000000         |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR<n>:TRIG:SOUR <trigger>``    | ``rp_GenTriggerSource``    | Set trigger source for selected signal.                                  |
| | Examples:                          |                            |                                                                          |
| | SOUR1:TRIG:SOUR EXT                |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR<n>:TRIG:IMM``               | ``rp_GenTrigger``          | Triggers selected source immediately.                                    |
| | Examples:                          |                            |                                                                          |
| | SOUR1:TRIG:IMM                     |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``TRIG:IMM``                       | ``rp_GenTrigger``          | Triggers both sources immediately.                                       |
| | Examples:                          |                            |                                                                          |
| | TRIG:IMM                           |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``GEN:RST``                        |                            | Reset generator to default settings.                                     |
| | Examples:                          |                            |                                                                          |
| | GEN:RST                            |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+

=======
Acquire
=======

Parameter options:

* ``<n> = {1,2}`` (set channel IN1 or IN2)

-------
Control
-------

.. tabularcolumns:: |p{28mm}|p{28mm}|p{28mm}|

+-----------------+-----------------+--------------------------------------------------------------+
| SCPI            | API             | description                                                  |
+=================+=================+==============================================================+
| | ``ACQ:START`` | ``rp_AcqStart`` | Starts acquisition.                                          |
| | Examples:     |                 |                                                              |
| | ACQ:START     |                 |                                                              |
+-----------------+-----------------+--------------------------------------------------------------+
| | ``ACQ:STOP``  | ``rp_AcqStop``  | Stops acquisition.                                           |
| | Examples:     |                 |                                                              |
| | ACQ:STOP      |                 |                                                              |
+-----------------+-----------------+--------------------------------------------------------------+
| | ``ACQ:RST``   | ``rp_AcqReset`` | Stops acquisition and sets all parameters to default values. |
| | Examples:     |                 |                                                              |
| | ACQ:STOP      |                 |                                                              |
+-----------------+-----------------+--------------------------------------------------------------+

--------------------------
Sampling rate & decimation
--------------------------

Parameter options:

* ``<decimation> = {1,8,64,1024,8192,65536}`` Default: ``1``
* ``<samplerate> = {125MHz, 15_6MHz, 1_9MHz, 103_8kHz, 15_2kHz, 1_9kHz}`` Default: ``125MHz``
* ``<average> = {OFF,ON}`` Default: ``ON``

.. tabularcolumns:: |p{28mm}|p{28mm}|p{28mm}|p{28mm}|

+--------------------------------------+-----------------------------+-----------------------------------+
| SCPI                                 | API                         | description                       |
+======================================+=============================+===================================+
| ``ACQ:DEC <decimation>``             | ``rp_AcqSetDecimation``     | Set decimation factor.            |
+--------------------------------------+-----------------------------+-----------------------------------+
| | ``ACQ:DEC?`` > ``<decimation>``    | ``rp_AcqGetDecimation``     | Get decimation factor.            |
| | Example:                           |                             |                                   |
| | ACQ:DEC?                           |                             |                                   |
+--------------------------------------+-----------------------------+-----------------------------------+
| ``ACQ:SRAT <par>``                   | ``rp_AcqSetSamplingRate``   | Set sampling rate.                |
+--------------------------------------+-----------------------------+-----------------------------------+
| | ``ACQ:SRAT?`` > ``<samplerate>``   | ``rp_AcqGetSamplingRate``   | Get sampling rate.                |
| | Example:                           |                             |                                   |
| | ACQ:SRAT?                          |                             |                                   |
+--------------------------------------+-----------------------------+-----------------------------------+
| | ``ACQ:SRA:HZ?``                    | ``rp_AcqGetSamplingRateHz`` | Get sampling rate in Hz.          |
| | Example:                           |                             |                                   |
| | ACQ:SRA:HZ?                        |                             |                                   |
| | Query return:                      |                             |                                   |
| | 125000000 Hz                       |                             |                                   |
+--------------------------------------+-----------------------------+-----------------------------------+
| | ``ACQ:AVG <par>``                  | ``rp_AcqSetAveraging``      | Enable/disable averaging.         |
+--------------------------------------+-----------------------------+-----------------------------------+
| | ``ACQ:AVG?``                       | ``rp_AcqGetAveraging``      | Get averaging status.             |
| | Example:                           |                             |                                   |
| | ACQ:AVG?                           |                             |                                   |
+--------------------------------------+-----------------------------+-----------------------------------+

=======
Trigger
=======

.. tabularcolumns:: |p{28mm}|p{28mm}|p{28mm}|p{28mm}|

+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| SCPI                                 | OPTIONS                   | DESCRIPTION                       | API                             |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | ``ACQ:TRIG <par>``                 | | <par>={DISABLED,NOW,    | Disable triggering, trigger       | ``rp_AcqSetTriggerSrc``         |
| |                                    |   CH1_PE,CH1_NE,CH2_PE,   | immediately or set trigger        |                                 |
| | Example:                           |   CH2_NE,EXT_PE,EXT_NE,   | source & edge.                    |                                 |
| | ACQ:TRIG CH1_PE                    |   AWG_PE, AWG_NE}         |                                   |                                 |
| |                                    | |                         |                                   |                                 |
| |                                    | | Default: DISABLED       |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | ``ACQ:TRIG:STAT?``                 |                           | Get trigger status.               | ``rp_AcqGetTriggerState``       |
| |                                    |                           |                                   |                                 |
| | Example:                           |                           |                                   | if DISABLED -> TD               |
| | ACQ:TRIG:STAT?                     |                           |                                   | else WAIT                       |
| |                                    |                           |                                   |                                 |
| | Query return:                      |                           |                                   |                                 |
| | {WAIT,TD}                          |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | ``ACQ:TRIG:DLY <par>``             | | <par>={value in         | Set trigger delay in samples.     | ``rp_AcqSetTriggerDelay``       |
| |                                    | | samples}                |                                   |                                 |
| | Example:                           | |                         |                                   |                                 |
| | ACQ:TRIG:DLY 2314                  | | Default: 0              |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | ``ACQ:TRIG:DLY?``                  |                           | Get trigger delay in samples.     | ``rp_AcqGetTriggerDelay``       |
| | Example:                           |                           |                                   |                                 |
| | ACQ:TRIG:DLY?                      |                           |                                   |                                 |
| |                                    |                           |                                   |                                 |
| | Query return:                      |                           |                                   |                                 |
| | 2314                               |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | ``ACQ:TRIG:DLY:NS <par>``          | <par>={value in ns}       | Set trigger delay in ns.          | ``rp_AcqSetTriggerDelayNs``     |
| |                                    |                           |                                   |                                 |
| | Example:                           | Default: 0                |                                   |                                 |
| | ACQ:TRIG:DLY:NS 128                |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | ``ACQ:TRIG:DLY:NS?``               |                           | Get trigger delay in ns.          | ``rp_AcqGetTriggerDelayNs``     |
| |                                    |                           |                                   |                                 |
| | Example:                           |                           |                                   |                                 |
| | ACQ:TRIG:DLY:NS?                   |                           |                                   |                                 |
| |                                    |                           |                                   |                                 |
| | Query return:                      |                           |                                   |                                 |
| | 128 ns                             |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | ``ACQ:SOUR<n>:GAIN <par>``         | <par>={LV,HV}             | Set gain settings to HIGH         | ``rp_AcqSetGain``               |
| |                                    |                           | or LOW. This gain is              |                                 |
| | Example:                           | Default: LV               | referring to jumper settings      |                                 |
| | ACQ:SOUR1:GAIN LV                  |                           | on Red Pitaya fast analog         |                                 |
| |                                    |                           | inputs.                           |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | ``ACQ:TRIG:LEV <par>``             | <par>={value in mV}       | Set trigger level in mV.          | ``rp_AcqSetChannelThreshold``   |
| |                                    |                           |                                   |                                 |
| | Example:                           | Default: 0                |                                   |                                 |
| | ACQ:TRIG:LEV 125 mV                |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | ``ACQ:TRIG:LEV?``                  |                           | Get trigger level in mV.          | ``rp_AcqGetChannelThreshold``   |
| |                                    |                           |                                   |                                 |
| | Example:                           |                           |                                   |                                 |
| | ACQ:TRIG:LEV?                      |                           |                                   |                                 |
| |                                    |                           |                                   |                                 |
| | Query return:                      |                           |                                   |                                 |
| | 123 mV                             |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+

=============
Data pointers
=============

.. tabularcolumns:: |p{28mm}|p{28mm}|p{28mm}|p{28mm}|

+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| SCPI                                 | OPTIONS                   | DESCRIPTION                       | API                             |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | ``ACQ:WPOS?``                      |                           | Returns current position of       | ``rp_AcqGetWritePointer``       |
| |                                    |                           | write pointer.                    |                                 |
| | Example:                           |                           |                                   |                                 |
| | ACQ:WPOS?                          |                           |                                   |                                 |
| |                                    |                           |                                   |                                 |
| | Query return:                      |                           |                                   |                                 |
| | {write pointer position}           |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | ``ACQ:TPOS?``                      |                           | Returns position where            | ``rp_AcqGetWritePointerAtTrig`` |
| |                                    |                           | trigger event appeared.           |                                 |
| | Example:                           |                           |                                   |                                 |
| | ACQ:TPOS?                          |                           |                                   |                                 |
| |                                    |                           |                                   |                                 |
| | Query return:                      |                           |                                   |                                 |
| | 1234                               |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+

=========
Data read
=========

.. tabularcolumns:: |p{28mm}|p{28mm}|p{28mm}|p{28mm}|

+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| SCPI                                 | OPTIONS                   | DESCRIPTION                       | API                             |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | ``ACQ:DATA:UNITS <PAR>``           | <par>={RAW, VOLTS}        | Selects units in which            | ``rp_AcqScpiDataUnits``         |
| |                                    |                           | acquired data will be             |                                 |
| | Example:                           | Default: VOLTS            | returned.                         |                                 |
| | ACQ:GET:DATA:UNITS RAW             |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | ``ACQ:DATA:FORMAT <PAR>``          | <par>={FLOAT, ASCII}      | Selects format acquired data      | ``rp_AcqScpiDataFormat``        |
| |                                    |                           | will be returned.                 |                                 |
| | Example:                           | Default: FLOAT            |                                   |                                 |
| | ACQ:GET:DATA:FORMAT ASCII          |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | ``ACQ:SOUR<n>:DATA:STA:END?``      | | <start_pos>             | Read samples from start to        | ``rp_AcqGetDataPosRaw``         |
| | ``<start_pos>,<end_pos>``          |   ={0,1,...,16384}        | stop position.                    | ``rp_AcqGetDataPosV``           |
| |                                    | |                         |                                   |                                 |
| | Example:                           | | <stop_pos>              |                                   |                                 |
| | ACQ:SOUR1:GET:DATA 10,13           |   ={0,1,...16384}         |                                   |                                 |
| |                                    | | stop_pos > start_pos    |                                   |                                 |
| | Query return:                      |                           |                                   |                                 |
| | {123,231,-231}                     |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | ``ACQ:SOUR<n>:DATA:STA:N?``        |                           | Read m samples from start         | ``rp_AcqGetDataRaw``            |
| | ``<start_pos>,<m>``                |                           | position on.                      | ``rp_AcqGetDataV``              |
| |                                    |                           |                                   |                                 |
| | Example:                           |                           |                                   |                                 |
| | ACQ:SOUR1:DATA? 10,3               |                           |                                   |                                 |
| |                                    |                           |                                   |                                 |
| | Query return:                      |                           |                                   |                                 |
| | {1.2,3.2,-1.2}                     |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | ``ACQ:SOUR<n>:DATA?``              |                           | Read full buf. size starting      | | ``rp_AcqGetOldestDataRaw``    |
| |                                    |                           | from oldest sample in             | | ``rp_AcqGetOldestDataV``      |
| | Example:                           |                           | buffer (this is first sample      | |                               |
| | ACQ:SOUR2:DATA?                    |                           | after trigger delay). Trigger     | | size=buf_size!                |
| |                                    |                           | delay by default is set to        |                                 |
| | Query return:                      |                           | zero (in samples or in            |                                 |
| | {1.2,3.2,...,-1.2}                 |                           | seconds). If trigger delay is     |                                 |
| |                                    |                           | set to zero it will read full     |                                 |
| |                                    |                           | buf. size starting from           |                                 |
| |                                    |                           | trigger.                          |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | ``ACQ:SOUR<n>:DATA:OLD:N?<m>``     |                           | Read m samples after              | ``rp_AcqGetOldestDataRaw``      |
| |                                    |                           | trigger delay, starting from      | ``rp_AcqGetOldestDataV``        |
| | Example:                           |                           | oldest sample in buffer (this     |                                 |
| | ACQ:SOUR2:DATA:OLD? 3              |                           | is first sample after trigger     |                                 |
| |                                    |                           | delay). Trigger delay by          |                                 |
| | Query return:                      |                           | default is set to zero (in        |                                 |
| | {1.2,3.2,-1.2}                     |                           | samples or in seconds). If        |                                 |
|                                      |                           | trigger delay is set to zero it   |                                 |
|                                      |                           | will read m samples starting      |                                 |
|                                      |                           | from trigger.                     |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | ``ACQ:SOUR<n>:DATA:LAT:N?<m>``     |                           | Read m samples before             | ``rp_AcqGetLatestDataRaw``      |
| |                                    |                           | trigger delay. Trigger delay      | ``rp_AcqGetLatestDataV``        |
| | Example:                           |                           | by default is set to zero (in     |                                 |
| | ACQ:SOUR1:DATA:LAT? 3              |                           | samples or in seconds). If        |                                 |
| |                                    |                           | trigger delay is set to zero it   |                                 |
| | Query return:                      |                           | will read m samples before        |                                 |
| | {1.2,3.2,-1.2}                     |                           | trigger.                          |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | ``ACQ:BUF:SIZE?``                  |                           |  Returns buffer size.             | ``rp_AcqGetBufSize``            |
| |                                    |                           |                                   |                                 |
| | Example:                           |                           |                                   |                                 |
| | ACQ:BUF:SIZE?                      |                           |                                   |                                 |
| |                                    |                           |                                   |                                 |
| | Query return:                      |                           |                                   |                                 |
| | 16384                              |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+ 
