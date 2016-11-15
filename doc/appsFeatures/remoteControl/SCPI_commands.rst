*******************************
List of supported SCPI commands 
*******************************

.. (link - https://dl.dropboxusercontent.com/s/eiihbzicmucjtlz/SCPI_commands_beta_release.pdf)

Table of correlated SCPI and API commands on Red Pitaya.

==============
LEDs and GPIOs
==============

Parameter options:

* ``<dir> = {OUT,IN}``
* ``<gpio> = {{DIO0_P...DIO7_P}, {DIO0_N...DIO7_N}}``
* ``<led> = {LED0...LED8}``
* ``<pin> = {gpio, led}``
* ``<state> = {0,1}``

.. tabularcolumns:: |p{28mm}|p{28mm}|p{28mm}|

+------------------------------------+-------------------------+------------------------------------------------------+
| SCPI                               | API                     | description                                          |
+====================================+=========================+======================================================+
| | ``DIG:PIN:DIR <dir>,<gpio>``     | ``rp_DpinSetDirection`` | Set direction of digital pins to output or input.    |
| | Examples:                        |                         |                                                      |                       
| | ``DIG:PIN:DIR OUT,DIO0_N``       |                         |                                                      |  
| | ``DIG:PIN:DIR IN,DIO1_P``        |                         |                                                      |                  
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
+=======================================+=====================+======================================================+
| | ``ANALOG:PIN <pin>,<value>``        | ``rp_ApinSetValue`` | | Set analog voltage on slow analog outputs.         |
| | Examples:                           |                     | | Voltage range of slow analog outputs is: 0 - 1.8 V |
| | ``ANALOG:PIN AOUT2,1.34``           |                     |                                                      |
+---------------------------------------+---------------------+------------------------------------------------------+
| | ``ANALOG:PIN? <pin>`` > ``<value>`` | ``rp_ApinGetValue`` | | Read analog voltage from slow analog inputs.       |
| | Examples:                           |                     | | Voltage range of slow analog inputs is: 0 3.3 V    |
| | ``ANALOG:PIN? AOUT2`` > ``1.34``    |                     |                                                      |
| | ``ANALOG:PIN? AIN1`` > ``1.12``     |                     |                                                      |
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
+======================================+============================+==========================================================================+
| | ``OUTPUT<n>:STATE <state>``        | | ``rp_GenOutEnable``      | Disable or enable fast analog outputs.                                   |
| | Examples:                          | | ``rp_GenOutDisable``     |                                                                          |
| | ``OUTPUT1:STATE ON``               |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR<n>:FREQ:FIX <frequency>``   | ``rp_GenFreq``             | Set frequency of fast analog outputs.                                    |
| | Examples:                          |                            |                                                                          |
| | ``SOUR2:FREQ:FIX 100000``          |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR<n>:FUNC <func>``            | ``rp_GenWaveform``         | Set waveform of fast analog outputs.                                     |
| | Examples:                          |                            |                                                                          |
| | ``SOUR2:FUNC TRIANGLE``            |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR<n>:VOLT <amplitude>``       | ``rp_GenAmp``              | | Set amplitude voltage of fast analog outputs.                          |
| | Examples:                          |                            | | Amplitude + offset value must be less than maximum output range +/- 1V |
| | ``SOUR2:VOLT 0.5``                 |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR<n>:VOLT:OFFS <offset>``     | ``rp_GenOffset``           | | Set offset voltage of fast analog outputs.                             |
| | Examples:                          |                            | | Amplitude + offset value must be less than maximum output range +/- 1V |
| | ``SOUR1:VOLT:OFFS 0.2``            |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR<n>:PHAS <phase>``           | ``rp_GenPhase``            | Set phase of fast analog outputs.                                        |
| | Examples:                          |                            |                                                                          |
| | ``SOUR2:PHAS 30``                  |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR<n>:DCYC <par>``             | ``rp_GenDutyCycle``        | Set duty cycle of PWM waveform.                                          |
| | Examples:                          |                            |                                                                          |
| | ``SOUR1:DCYC 20``                  |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR<n>:TRAC:DATA:DATA <array>`` | ``rp_GenArbWaveform``      | Import data for arbitrary waveform generation.                           |
| | Examples:                          |                            |                                                                          |
| | ``SOUR1:TRAC:DATA:DATA``           |                            |                                                                          |
| | ``1,0.5,0.2``                      |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR<n>:BURS:STAT <burst>``      | ``rp_GenMode``             | Enable or disable burst (pulse) mode.                                    |
| | Examples:                          |                            | Red Pitaya will generate **R** number of **N** periods of signal         |
| | ``SOUR1:BURS:STAT ON``             |                            | and then stop. Time between bursts is **P**.                             |
| | ``SOUR1:BURS:STAT OFF``            |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR<n>:BURS:NCYC <count>``      | ``rp_GenBurstCount``       | Set N number of periods in one burst.                                    |
| | Examples:                          |                            |                                                                          |
| | ``SOUR1:BURS:NCYC 3``              |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR1:BURS:NOR <count>``         | ``rp_GenBurstRepetitions`` | Set R number of repeated bursts.                                         |
| | Examples:                          |                            |                                                                          |
| | ``SOUR1:BURS:NOR 5``               |                            |                                                                          |
+--------------------------------------+----------------------------+---------------------------+----------------------------------------------+
| | ``SOUR1:BURS:INT:PER <time>``      | ``rp_GenBurstPeriod``      | Set P total time of one burst in in micro seconds.                       |
| | Examples:                          |                            | This includes the signal and delay.                                      |
| | ``SOUR1:BURS:INT:PER 1000000``     |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR<n>:TRIG:SOUR <trigger>``    | ``rp_GenTriggerSource``    | Set trigger source for selected signal.                                  |
| | Examples:                          |                            |                                                                          |
| | ``SOUR1:TRIG:SOUR EXT``            |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``SOUR<n>:TRIG:IMM``               | ``rp_GenTrigger``          | Triggers selected source immediately.                                    |
| | Examples:                          |                            |                                                                          |
| | ``SOUR1:TRIG:IMM``                 |                            |                                                                          |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``TRIG:IMM``                       | ``rp_GenTrigger``          | Triggers both sources immediately.                                       |
+--------------------------------------+----------------------------+--------------------------------------------------------------------------+
| | ``GEN:RST``                        |                            | Reset generator to default settings.                                     |
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
+-----------------+-----------------+--------------------------------------------------------------+
| | ``ACQ:STOP``  | ``rp_AcqStop``  | Stops acquisition.                                           |
+-----------------+-----------------+--------------------------------------------------------------+
| | ``ACQ:RST``   | ``rp_AcqReset`` | Stops acquisition and sets all parameters to default values. |
+-----------------+-----------------+--------------------------------------------------------------+

--------------------------
Sampling rate & decimation
--------------------------

Parameter options:

* ``<decimation> = {1,8,64,1024,8192,65536}`` Default: ``1``
* ``<samplerate> = {125MHz, 15_6MHz, 1_9MHz, 103_8kHz, 15_2kHz, 1_9kHz}`` Default: ``125MHz``
* ``<average> = {OFF,ON}`` Default: ``ON``

.. tabularcolumns:: |p{28mm}|p{28mm}|p{28mm}|

+-------------------------------------+-----------------------------+-----------------------------------+
| SCPI                                | API                         | description                       |
+=====================================+=============================+===================================+
| ``ACQ:DEC <decimation>``            | ``rp_AcqSetDecimation``     | Set decimation factor.            |
+-------------------------------------+-----------------------------+-----------------------------------+
| | ``ACQ:DEC?`` > ``<decimation>``   | ``rp_AcqGetDecimation``     | Get decimation factor.            |
| | Example:                          |                             |                                   |
| | ``ACQ:DEC?`` > ``1``              |                             |                                   |
+-------------------------------------+-----------------------------+-----------------------------------+
| | ``ACQ:AVG <average>``             | ``rp_AcqSetAveraging``      | Enable/disable averaging.         |
+-------------------------------------+-----------------------------+-----------------------------------+
| | ``ACQ:AVG?`` > ``<average>``      | ``rp_AcqGetAveraging``      | Get averaging status.             |
| | Example:                          |                             |                                   |
| | ``ACQ:AVG?`` > ``ON``             |                             |                                   |
+-------------------------------------+-----------------------------+-----------------------------------+

=======
Trigger
=======

Parameter options:

* ``<source> = {DISABLED, NOW, CH1_PE, CH1_NE, CH2_PE, CH2_NE, EXT_PE, EXT_NE, AWG_PE, AWG_NE}``  Default: ``DISABLED``
* ``<status> = {WAIT, TD}``
* ``<time> = {value in ns}``
* ``<counetr> = {value in samples}``
* ``<gain> = {LV, HV}``
* ``<level> = {value in mV}``

.. tabularcolumns:: |p{28mm}|p{28mm}|p{28mm}|

+-------------------------------------+-------------------------------+-----------------------------------------------------------------------------+
| SCPI                                | API                           | DESCRIPTION                                                                 |
+=====================================+===============================+=============================================================================+
| | ``ACQ:TRIG <source>``             | ``rp_AcqSetTriggerSrc``       | Disable triggering, trigger immediately or set trigger source & edge.       |
| | Example:                          |                               |                                                                             |
| | ``ACQ:TRIG CH1_PE``               |                               |                                                                             |
+-------------------------------------+-------------------------------+-----------------------------------------------------------------------------+
| | ``ACQ:TRIG:STAT?``                | ``rp_AcqGetTriggerState``     | Get trigger status. If DISABLED -> TD else WAIT.                            |
| | Example:                          |                               |                                                                             |
| | ``ACQ:TRIG:STAT?`` > ``WAIT``     |                               |                                                                             |
+-------------------------------------+-------------------------------+-----------------------------------------------------------------------------+
| | ``ACQ:TRIG:DLY <time>``           | ``rp_AcqSetTriggerDelay``     | Set trigger delay in samples.                                               |
| | Example:                          |                               |                                                                             |
| | ``ACQ:TRIG:DLY 2314``             |                               |                                                                             |
+-------------------------------------+-------------------------------+-----------------------------------------------------------------------------+
| | ``ACQ:TRIG:DLY?`` > ``<time>``    | ``rp_AcqGetTriggerDelay``     | Get trigger delay in samples.                                               |
| | Example:                          |                               |                                                                             |
| | ``ACQ:TRIG:DLY?`` > ``2314``      |                               |                                                                             |
+-------------------------------------+-------------------------------+-----------------------------------------------------------------------------+
| | ``ACQ:TRIG:DLY:NS <time>``        | ``rp_AcqSetTriggerDelayNs``   | Set trigger delay in ns.                                                    |
| | Example:                          |                               |                                                                             |
| | ``ACQ:TRIG:DLY:NS 128``           |                               |                                                                             |
+-------------------------------------+-------------------------------+-----------------------------------------------------------------------------+
| | ``ACQ:TRIG:DLY:NS?`` > ``<time>`` | ``rp_AcqGetTriggerDelayNs``   | Get trigger delay in ns.                                                    |
| | Example:                          |                               |                                                                             |
| | ``ACQ:TRIG:DLY:NS?`` > ``128ns``  |                               |                                                                             |
+-------------------------------------+-------------------------------+-----------------------------------------------------------------------------+
| | ``ACQ:SOUR<n>:GAIN <gain>``       | ``rp_AcqSetGain``             | Set gain settings to HIGH or LOW.                                           |
| | Example:                          |                               | This gain is referring to jumper settings on Red Pitaya fast analog inputs. |
| | ``ACQ:SOUR1:GAIN LV``             |                               |                                                                             |
+-------------------------------------+-------------------------------+-----------------------------------------------------------------------------+
| | ``ACQ:TRIG:LEV <level>``          | ``rp_AcqSetChannelThreshold`` | Set trigger level in mV.                                                    |
| | Example:                          |                               |                                                                             |
| | ``ACQ:TRIG:LEV 125 mV``           |                               |                                                                             |
+-------------------------------------+-------------------------------+-----------------------------------------------------------------------------+
| | ``ACQ:TRIG:LEV?`` > ``level``     | ``rp_AcqGetChannelThreshold`` | Get trigger level in mV.                                                    |
| | Example:                          |                               |                                                                             |
| | ``ACQ:TRIG:LEV?`` > ``123 mV``    |                               |                                                                             |
+-------------------------------------+-------------------------------+-----------------------------------------------------------------------------+

=============
Data pointers
=============

Parameter options:

* ``<pos> = {position inside circular buffer}``

.. tabularcolumns:: |p{28mm}|p{28mm}|p{28mm}|p{28mm}|

+------------------------------+---------------------------------+------------------------------------------------+
| SCPI                         | API                             | DESCRIPTION                                    |
+------------------------------+---------------------------------+------------------------------------------------+
| | ``ACQ:WPOS?`` > ``pos``    | ``rp_AcqGetWritePointer``       | Returns current position of write pointer.     |
| | Example:                   |                                 |                                                |
| | ``ACQ:WPOS?`` > ``1024``   |                                 |                                                |
+------------------------------+---------------------------------+------------------------------------------------+
| | ``ACQ:TPOS?`` > ``pos``    | ``rp_AcqGetWritePointerAtTrig`` | Returns position where trigger event appeared. |
| | Example:                   |                                 |                                                |
| | ``ACQ:TPOS?`` > ``512``    |                                 |                                                |
+------------------------------+---------------------------------+------------------------------------------------+

=========
Data read
=========


* ``<units> = {RAW, VOLTS}``
* ``<format> = {FLOAT, ASCII}`` Default ``FLOAT``

.. tabularcolumns:: |p{28mm}|p{28mm}|p{28mm}|

+-----------------------------------+------------------------------+------------------------------------------------------------------------------------------+
| SCPI                              | API                          | DESCRIPTION                                                                              |
+-----------------------------------+------------------------------+------------------------------------------------------------------------------------------+
| | ``ACQ:DATA:UNITS <units>``      | ``rp_AcqScpiDataUnits``      | Selects units in which acquired data will be returned.                                   |
| | Example:                        |                              |                                                                                          |
| | ``ACQ:GET:DATA:UNITS RAW``      |                              |                                                                                          |
+-----------------------------------+------------------------------+------------------------------------------------------------------------------------------+
| | ``ACQ:DATA:FORMAT <format>``    | ``rp_AcqScpiDataFormat``     | Selects format acquired data will be returned.                                           |
| | Example:                        |                              |                                                                                          |
| | ``ACQ:GET:DATA:FORMAT ASCII``   |                              |                                                                                          |
+-----------------------------------+------------------------------+------------------------------------------------------------------------------------------+
| | ``ACQ:SOUR<n>:DATA:STA:END?`` > | | ``rp_AcqGetDataPosRaw``    | | Read samples from start to stop position.                                              |
| | ``<start_pos>,<end_pos>``       | | ``rp_AcqGetDataPosV``      | | ``<start_pos> = {0,1,...,16384}``                                                      |
| | Example:                        |                              | | ``<stop_pos> = {0,1,...116384}``                                                       |
| | ``ACQ:SOUR1:GET:DATA 10,13`` >  |                              |                                                                                          |
| | ``{123,231,-231}``              |                              |                                                                                          |
+-----------------------------------+------------------------------+------------------------------------------------------------------------------------------+
| | ``ACQ:SOUR<n>:DATA:STA:N?``     | | ``rp_AcqGetDataRaw``       |  Read ``m`` samples from start position on.                                              |
| | ``<start_pos>,<m>`` > ``...``   | | ``rp_AcqGetDataV``         |                                                                                          |
| | Example:                        |                              |                                                                                          |
| | ``ACQ:SOUR1:DATA? 10,3`` >      |                              |                                                                                          |
| | ``{1.2,3.2,-1.2}``              |                              |                                                                                          |
+-----------------------------------+------------------------------+------------------------------------------------------------------------------------------+
| | ``ACQ:SOUR<n>:DATA?``           | | ``rp_AcqGetOldestDataRaw`` | | Read full buf.                                                                         |
| | Example:                        | | ``rp_AcqGetOldestDataV``   | | Size starting from oldest sample in buffer (this is first sample after trigger delay). |
| | ``ACQ:SOUR2:DATA?`` >           |                              | | Trigger delay by default is set to zero (in samples or in seconds).                    |
| | ``{1.2,3.2,...,-1.2}``          |                              | | If trigger delay is set to zero it will read full buf. size starting from trigger.     |
+-----------------------------------+------------------------------+------------------------------------------------------------------------------------------+
| | ``ACQ:SOUR<n>:DATA:OLD:N?<m>``  | | ``rp_AcqGetOldestDataRaw`` | | Read m samples after trigger delay, starting from oldest sample in buffer              |
| | Example:                        | | ``rp_AcqGetOldestDataV``   | | (this is first sample after trigger delay).                                            |
| | ``ACQ:SOUR2:DATA:OLD? 3`` >     |                              | | Trigger delay by default is set to zero (in samples or in seconds).                    |
| | ``{1.2,3.2,-1.2}``              |                              | | If trigger delay is set to zero it will read m samples starting from trigger.          |
+-----------------------------------+------------------------------+------------------------------------------------------------------------------------------+
| | ``ACQ:SOUR<n>:DATA:LAT:N?<m>``  | | ``rp_AcqGetLatestDataRaw`` | | Read ``m`` samples before trigger delay.                                               |
| | Example:                        | | ``rp_AcqGetLatestDataV``   | | Trigger delay by default is set to zero (in samples or in seconds).                    |
| | ``ACQ:SOUR1:DATA:LAT? 3`` >     |                              | | If trigger delay is set to zero it will read m samples before trigger.                 |
| | ``{1.2,3.2,-1.2}``              |                              |                                                                                          |
+-----------------------------------+------------------------------+------------------------------------------------------------------------------------------+
| | ``ACQ:BUF:SIZE?`` > ``<size>``  | ``rp_AcqGetBufSize``         |  Returns buffer size.                                                                    |
| | Example:                        |                              |                                                                                          |
| | ``ACQ:BUF:SIZE?`` > ``16384``   |                              |                                                                                          |
+-----------------------------------+------------------------------+------------------------------------------------------------------------------------------+ 
