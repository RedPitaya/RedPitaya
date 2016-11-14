Remote control
##############

.. https://owncloud.redpitaya.com/index.php/apps/files/?dir=%2FWEB%20page%2Fapps%2FSCPI

.. image:: SCPI_web_lr.png

The STEMLab board can be controlled remotely over LAN or wireless interface using Matlab, Labview, Scilab or Python via the Red Pitaya SCPI (Standard Commands for Programmable Instrumentation) list of commands. The SCPI interface/environment is commonly used to control T&M instruments for development, research or test automation purposes. SCPI uses a set of SCPI commands that are recognized by the instruments to enable specific actions to be taken (e.g.: acquiring data from fast analog inputs, generating signals and controlling other periphery of the Red Pitaya STEMLab platform). The SCPI commands are extremely useful when complex signal analysis is required where SW environment such as MATLAB provides powerful data analysis tools and SCPI commands simple access to raw data acquired on STEMLab board.

**Features**

    - Quickly write control routines and programs using Matlab, Labview, Scilab or Python
    - Use powerful data analysis tools of Matlab, Labview, Scilab or Python to analyze raw signals
      acquired by the STEMLab board
    - Write testing scripts and routines
    - Incorporate your STEMLab and Labview into testing and production lines
    - Take quick measurements directly with your PC 

With SCPI commands you will be able to control all STEMlab features such us 
Digital Inputs/Outputs, 
Digital Communication interfaces (I2C, SPI, UART), 
Slow Analog Inputs/Outputs & 
Fast Analog Inputs/Outputs.
            
quick start
***********

.. TODO oblikuj

**Connect to your Red Pitaya remotely**

Assuming you have successfully connected to your Red Pitaya using :ref: these instructions.

Remotely connect using Putty on Windows machines or with SSH using Terminal on UNIX(OSX/Linux) machines.

In this example we have our Red Pitaya connected using a WIFI dongle directly to our computer (our Pitayas' IP is therefore 192.168.128.1).

By default the username is root and password is root.

.. TODO - page missing
                how to start server from WEB interface or manualy from shell
                    dodam notri v dokument (http://redpitaya.com/control/?with=matlab)
                    WEB (zumi)
                
                how to start controling Red Pitaya using
                    (http://redpitaya.com/control/?with=matlab spodaj)
                    MATLAB  ( zumi )
                    LABVIEW ( zumi )
                    Python  ( zumi )
                    SCILAB  ( zumi )
                exampli (link - http://redpitaya.com/examples-new/ )

List of supported SCPI commands 
*******************************

.. (link - https://dl.dropboxusercontent.com/s/eiihbzicmucjtlz/SCPI_commands_beta_release.pdf)

Table of correlated SCPI and API commands on Red Pitaya.

.. tabularcolumns:: |p{28mm}|p{28mm}|p{28mm}|p{28mm}|

+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| SCPI                                 | OPTIONS                   | DESCRIPTION                       | API                             |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| **LED diodes and GPIOs Red Pitaya**                                                                                                    |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **DIG:PIN:DIR <dir>,<pin>**        | | <dir> = {OUTP,INP}      | Set direction of digital pins     | **rp_DpinSetDirection**         |
| |                                    | | <pin>={DIO1_P...DIO7_P, | to output or input.               |                                 |                                                 
| | Examples:                          | | DIO0_N...DIO7_N}        |                                   |                                 |                       
| | DIG:PIN:DIR OUTP,DIO0_N            | |                         |                                   |                                 |  
| | DIG:PIN:DIR INP,DIO1_P             | | OUTP = OUTPUT           |                                   |                                 |                  
| |                                    | | INP = INPUT             |                                   |                                 |                
| |                                    | | Default: OUTP           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **DIG:PIN <pin>,<state>**          | | <pin>={DIO1_P...DIO7_P, | Set state of digital outputs      | **rp_DpinSetState**             |
| |                                    | | DIO0_N...DIO7_N,        | to 1(HIGH) or 0(LOW).             |                                 |
| | Examples:                          | | LED1...LED8}            |                                   |                                 |
| | DIG:PIN DIO0_N,1                   | | <state>={0,1}           |                                   |                                 |
| | DIG:PIN LED2,1                     | |                         |                                   |                                 |
| |                                    | | Default: 0              |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **DIG:PIN? <pin>**                 | | <pin>={DIO1_P...DIO7_P, | Get state of digital inputs       | **rp_DpinGetState**             |
| |                                    | | DIO0_N...DIO7_N,        | and outputs.                      |                                 |
| | Examples:                          | | LED1...LED8}            |                                   |                                 |
| | DIG:PIN? DIO0_N                    | |                         |                                   |                                 |
| | DIG:PIN? LED2                      | |                         |                                   |                                 |
| | Query return:                      | |                         |                                   |                                 |
| | {0, 1, ERR}                        | |                         |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+


.. tabularcolumns:: |p{28mm}|p{28mm}|p{28mm}|p{28mm}|

+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| SCPI                                 | OPTIONS                   | DESCRIPTION                       | API                             |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| **Analog Inputs and Outputs**                                                                                                          |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ANALOG:PIN <pin>,<value>**       | | <pin>={AOUT0, AOUT1,    | | Set analog voltage on           |  **rp_ApinSetValue**            |
| |                                    | | AOUT2, AOUT3}           | | slow analog outputs.            |                                 |
| | Examples:                          | | <value>={value in       | |                                 |                                 |
| | ANALOG:PIN AOUT0,1                 | | Volts}                  | | Voltage range of slow           |                                 |
| | ANALOG:PIN AOUT2,1.34              | |                         | | analog outputs is:              |                                 |
| |                                    | | Default: 0              | | 0 1.8 V                         |                                 |
| |                                    | |                         | |                                 |                                 |
| |                                    | |                         | |                                 |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ANALOG:PIN? <pin>**              |  <pin>={AIN0, AIN1,       | Read analog voltage               | **rp_ApinGetValue**             |
| |                                    |  AIN2, AIN3, AOUT0,       | from slow analog inputs.          |                                 |
| | Examples:                          |  AOUT1, AOUT2, AOUT3}     | Voltage range of slow             |                                 |
| | ANALOG:PIN? AOUT0                  |                           | analog inputs is:                 |                                 |
| | ANALOG:PIN? AIN2                   |                           | 0 3.3 V                           |                                 |
| |                                    |                           |                                   |                                 |
| | Query return:                      |                           |                                   |                                 |
| | {value in Volts, ERR}              |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+

.. tabularcolumns:: |p{28mm}|p{28mm}|p{28mm}|p{28mm}|

+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| SCPI                                 | OPTIONS                   | DESCRIPTION                       | API                             |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **Signal Generator**                                                                                                                 |
| | **<n> = {1,2} (set channel OUT1 or OUT2)**                                                                                           |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **OUTPUT<n>:STATE <par>**          | | <par>={ON,OFF}          | Disable or enable fast            | **rp_GenOutEnable**             |
| |                                    | |                         | analog outputs.                   | **rp_GenOutDisable**            |
| | Examples:                          | | Default: OFF            |                                   |                                 |
| | OUTPUT1:STATE ON                   | |                         |                                   |                                 |
| | OUTPUT2:STATE OFF                  | |                         |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **SOUR<n>:FREQ:FIX <value>**       | | <value>={frequency      | Set frequency of fast analog      | **rp_GenFreq**                  |
| |                                    | | 0Hz62.5e6Hz}            | outputs.                          |                                 |
| | Examples:                          | |                         |                                   |                                 |
| | SOUR1:FREQ:FIX 1000                | | Default: 1000           |                                   |                                 |
| | SOUR2:FREQ:FIX 100000              | |                         |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **SOUR<n>:FUNC <par>**             | |  <par>={SINE, SQUARE,   | Set waveform of fast analog       | **rp_GenWaveform**              |
| |                                    | |  TRIANGLE, SAWU, SAWD   | outputs.                          |                                 |
| | Examples:                          | |  PWM, ARBITRARY}        |                                   |                                 |
| | SOUR1:FUNC SINE                    | |                         |                                   |                                 |
| | SOUR2:FUNC TRIANGLE                | |  Default: SINE          |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **SOUR<n>:VOLT <value>**           | | <value>={amplitude 1V   | Set amplitude voltage of          | **rp_GenAmp**                   |
| |                                    | |  1V}                    | fast analog outputs.              |                                 |
| | Examples:                          | |                         | Amplitude + offset value          |                                 |
| | SOUR1:VOLT 1                       | | Default: 1              | must be less than maximum         |                                 |
| | SOUR2:VOLT 0.5                     | |                         | output range +/ 1V                |                                 |
| |                                    | | AMP+OFFS <= \|1\|V      |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **SOUR<n>:VOLT:OFFS <value>**      | | <value>={offset -1V     | Set offset voltage of fast        | **rp_GenOffset**                |
| |                                    | | -1V}                    | analog outputs. Amplitude         |                                 |
| | Examples:                          | |                         | + offset value must be less       |                                 |
| | SOUR1:VOLT:OFFS 0.2                | | Default: 0              | than maximum output range         |                                 |
| | SOUR1:VOLT:OFFS 0.1                | |                         | +/ 1V                             |                                 |
| |                                    | | AMP+OFFS <= \|1\|V      |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **SOUR<n>:PHAS <value>**           | | <value>={phase 360deg   | Set phase of fast analog          | **rp_GenPhase**                 |
| |                                    |    360deg}                | outputs.                          |                                 |
| | Examples:                          | |                         |                                   |                                 |
| | SOUR2:PHAS 30                      | | Default: 0              |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **SOUR<n>:DCYC <par>**             | | <value>={duty cycle     | Set duty cycle of PWM             | **rp_GenDutyCycle**             |
| |                                    |   0100}                   | waveform.                         |                                 |
| | Examples:                          | |                         |                                   |                                 |
| | SOUR1:DCYC 34                      | | Default: 50             |                                   |                                 |
| | SOUR2:DCYC 50                      | |                         |                                   |                                 |
| |                                    | | Only for PWM            |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **SOUR<n>:TRAC:DATA:DATA <array>** | | <array>={value1,        | Import data for arbitrary         | **rp_GenArbWaveform**           |
| |                                    |   value2,...valueN}       | waveform generation.              |                                 |
| | Examples:                          |   max. 16k values         |                                   |                                 |
| | SOUR1:TRAC:DATA:DATA               | |                         |                                   |                                 |
| | 1,0.5,0.2                          | | Values are floats in    |                                   |                                 |
| |                                    |   range from -1 to 1.     |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **SOUR<n>:BURS:STAT <par>**        | | <par>={ON,OFF}          | Enable or disable                 | **rp_GenMode**                  |
| |                                    | |                         | burst (pulse) mode.               |                                 |
| | Examples:                          | | Default: OFF            | Red Pitaya will generate          |                                 |
| | SOUR1:BURS:STAT ON                 | |                         | Rtimes N periods of signal        |                                 |
| | SOUR1:BURS:STAT OFF                | |                         | and then stop. Time               |                                 |
| |                                    | |                         | between is P.                     |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **SOUR<n>:BURS:NCYC <value>**      | | <value>={burst count    | Set N number of generated         | **rp_GenBurstCount**            |
| |                                    |   150000, INF}            | signals in one burst              |                                 |
| |                                    | |                         |                                   |                                 |
| |                                    | | INF = infinity -        |                                   |                                 |
| |                                    |   continuous              |                                   |                                 |
| | Examples:                          | |                         |                                   |                                 |
| | SOUR1:BURS:NCYC 3                  | | Default: 1              |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **SOUR1:BURS:NOR <value>**         | | <value>={burst          | Set R number of repeated bursts.  | **rp_GenBurstRepetitions**      |
| |                                    | | repetitions 150000,     |                                   |                                 |
| | Examples:                          | | INF}                    |                                   |                                 |
| | SOUR1:BURS:NOR 5                   | |                         |                                   |                                 |
| |                                    | | INF = infinity          |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **SOUR1:BURS:INT:PER <value>**     | <value>={bust period      | Set P total time of one burst     | **rp_GenBurstPeriod**           |
| |                                    | 1us500s}                  | in micro seconds. This            |                                 |
| | Examples:                          |                           | includes the signal and           |                                 |
| | SOUR1:BURS:INT:PER 1000000         |                           | delay.                            |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **SOUR<n>:TRIG:SOUR <par>**        | <par>={EXT_PE,EXT_NE,IN   | Set trigger source for            | **rp_GenTriggerSource**         |
| |                                    | T, GATED}                 | selected signal.                  |                                 |
| | Examples:                          |                           |                                   |                                 |
| | SOUR1:TRIG:SOUR EXT                | EXT = External            |                                   |                                 |
| |                                    | INT = Internal            |                                   |                                 |
| |                                    | GATED = gated busts       |                                   |                                 |
| |                                    |                           |                                   |                                 |
| |                                    | Default: INT              |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **SOUR<n>:TRIG:IMM**               |                           | Triggers selected source          | **rp_GenTrigger**               |
| |                                    |                           | immediately.                      |                                 |
| | Examples:                          |                           |                                   |                                 |
| | SOUR1:TRIG:IMM                     |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **TRIG:IMM**                       |                           | Triggers both sources             | **rp_GenTrigger**               |
| |                                    |                           | immediately.                      |                                 |
| | Examples:                          |                           |                                   |                                 |
| | TRIG:IMM                           |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **GEN:RST**                        |                           | Reset generator to default        |                                 |
| |                                    |                           | settings.                         |                                 |
| | Examples:                          |                           |                                   |                                 |
| | GEN:RST                            |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+


.. tabularcolumns:: |p{28mm}|p{28mm}|p{28mm}|p{28mm}|

+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| SCPI                                 | OPTIONS                   | DESCRIPTION                       | API                             |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| Acquire                                                                                                                                |
| <n> = {1,2} (set channel IN1 or IN2)                                                                                                   |
| Control                                                                                                                                |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:START**                      |                           | Starts acquisition.               | **rp_AcqStart**                 |
| |                                    |                           |                                   |                                 |
| | Examples:                          |                           |                                   |                                 |
| | ACQ:START                          |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:STOP**                       |                           | Stops acquisition.                | **rp_AcqStop**                  |
| |                                    |                           |                                   |                                 |
| | Examples:                          |                           |                                   |                                 |
| | ACQ:STOP                           |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:RST**                        |                           | Stops acquisition and sets        | **rp_AcqReset**                 |
| |                                    |                           | all parameters to default         |                                 |
| | Examples:                          |                           | values.                           |                                 |
| | ACQ:STOP                           |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| Sampling rate & decimation                                                                                                             |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| **ACQ:DEC <par>**                    | <par>={1,8,64,1024,8192,  | Set decimation factor.            | **rp_AcqSetDecimation**         |
|                                      | 65536}                    |                                   |                                 |
|                                      |                           |                                   |                                 |
|                                      | Default: 1                |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:DEC?**                       |                           | Get decimation factor.            | **rp_AcqGetDecimation**         |
| |                                    |                           |                                   |                                 |
| | Example:                           |                           |                                   |                                 |
| | ACQ:DEC?                           |                           |                                   |                                 |
| |                                    |                           |                                   |                                 |
| | Query return:                      |                           |                                   |                                 |
| | {1,8,64,1024,8192,65536}           |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| **ACQ:SRAT <par>**                   | | <par>={125MHz,15_6MHz,  | Set sampling rate.                | **rp_AcqSetSamplingRate**       |
|                                      | | 1_9MHz,103_8kHz,        |                                   |                                 |
|                                      | | 15_2kHz, 1_9kHz}        |                                   |                                 |
|                                      | |                         |                                   |                                 |
|                                      | | Default: 125MHz         |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:SRAT?**                      |                           | Get sampling rate.                | **rp_AcqGetSamplingRate**       |
| |                                    |                           |                                   |                                 |
| | Example:                           |                           |                                   |                                 |
| | ACQ:SRAT?                          |                           |                                   |                                 |
| | Query return:                      |                           |                                   |                                 |
| | {125MHz,15_6MHz,                   |                           |                                   |                                 |
| | 1_9MHz,103_8kHz, 15_2kHz,          |                           |                                   |                                 |
| | 1_9kHz}                            |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:SRA:HZ?**                    |                           | Get sampling rate in Hz.          | **rp_AcqGetSamplingRateHz**     |
| |                                    |                           |                                   |                                 |
| | Example:                           |                           |                                   |                                 |
| | ACQ:SRA:HZ?                        |                           |                                   |                                 |
| |                                    |                           |                                   |                                 |
| | Query return:                      |                           |                                   |                                 |
| | 125000000 Hz                       |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:AVG <par>**                  | | <par>={OFF,ON}          | Enable/disable averaging.         | **rp_AcqSetAveraging**          |
| |                                    | |                         |                                   |                                 |
| |                                    | | Default: ON             |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:AVG?**                       |                           | Get averaging status.             | **rp_AcqGetAveraging**          |
| |                                    |                           |                                   |                                 |
| | Example:                           |                           |                                   |                                 |
| | ACQ:AVG?                           |                           |                                   |                                 |
| |                                    |                           |                                   |                                 |
| | Query return:                      |                           |                                   |                                 |
| | {OFF,ON}                           |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+


.. tabularcolumns:: |p{28mm}|p{28mm}|p{28mm}|p{28mm}|

+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| SCPI                                 | OPTIONS                   | DESCRIPTION                       | API                             |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| Trigger                                                                                                                                |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:TRIG <par>**                 | | <par>={DISABLED,NOW,    | Disable triggering, trigger       | **rp_AcqSetTriggerSrc**         |
| |                                    |   CH1_PE,CH1_NE,CH2_PE,   | immediately or set trigger        |                                 |
| | Example:                           |   CH2_NE,EXT_PE,EXT_NE,   | source & edge.                    |                                 |
| | ACQ:TRIG CH1_PE                    |   AWG_PE, AWG_NE}         |                                   |                                 |
| |                                    | |                         |                                   |                                 |
| |                                    | | Default: DISABLED       |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:TRIG:STAT?**                 |                           | Get trigger status.               | **rp_AcqGetTriggerState**       |
| |                                    |                           |                                   |                                 |
| | Example:                           |                           |                                   | if DISABLED -> TD               |
| | ACQ:TRIG:STAT?                     |                           |                                   | else WAIT                       |
| |                                    |                           |                                   |                                 |
| | Query return:                      |                           |                                   |                                 |
| | {WAIT,TD}                          |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:TRIG:DLY <par>**             | | <par>={value in         | Set trigger delay in samples.     | **rp_AcqSetTriggerDelay**       |
| |                                    | | samples}                |                                   |                                 |
| | Example:                           | |                         |                                   |                                 |
| | ACQ:TRIG:DLY 2314                  | | Default: 0              |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:TRIG:DLY?**                  |                           | | Get trigger delay in            | **rp_AcqGetTriggerDelay**       |
| |                                    |                           |   samples.                        |                                 |
| | Example:                           |                           | |                                 |                                 |
| | ACQ:TRIG:DLY?                      |                           | |                                 |                                 |
| |                                    |                           | |                                 |                                 |
| | Query return:                      |                           | |                                 |                                 |
| | 2314                               |                           | |                                 |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:TRIG:DLY:NS <par>**          | <par>={value in ns}       | Set trigger delay in ns.          | **rp_AcqSetTriggerDelayNs**     |
| |                                    |                           |                                   |                                 |
| | Example:                           | Default: 0                |                                   |                                 |
| | ACQ:TRIG:DLY:NS 128                |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:TRIG:DLY:NS?**               |                           | Get trigger delay in ns.          | **rp_AcqGetTriggerDelayNs**     |
| |                                    |                           |                                   |                                 |
| | Example:                           |                           |                                   |                                 |
| | ACQ:TRIG:DLY:NS?                   |                           |                                   |                                 |
| |                                    |                           |                                   |                                 |
| | Query return:                      |                           |                                   |                                 |
| | 128 ns                             |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:SOUR<n>:GAIN <par>**         | <par>={LV,HV}             | Set gain settings to HIGH         | **rp_AcqSetGain**               |
| |                                    |                           | or LOW. This gain is              |                                 |
| | Example:                           | Default: LV               | referring to jumper settings      |                                 |
| | ACQ:SOUR1:GAIN LV                  |                           | on Red Pitaya fast analog         |                                 |
| |                                    |                           | inputs.                           |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:TRIG:LEV <par>**             | <par>={value in mV}       | Set trigger level in mV.          | **rp_AcqSetChannelThreshold**   |
| |                                    |                           |                                   |                                 |
| | Example:                           | Default: 0                |                                   |                                 |
| | ACQ:TRIG:LEV 125 mV                |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:TRIG:LEV?**                  |                           | Get trigger level in mV.          | **rp_AcqGetChannelThreshold**   |
| |                                    |                           |                                   |                                 |
| | Example:                           |                           |                                   |                                 |
| | ACQ:TRIG:LEV?                      |                           |                                   |                                 |
| |                                    |                           |                                   |                                 |
| | Query return:                      |                           |                                   |                                 |
| | 123 mV                             |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+

.. tabularcolumns:: |p{28mm}|p{28mm}|p{28mm}|p{28mm}|

+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| SCPI                                 | OPTIONS                   | DESCRIPTION                       | API                             |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| **Data pointers**                                                                                                                      |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:WPOS?**                      |                           | Returns current position of       | **rp_AcqGetWritePointer**       |
| |                                    |                           | write pointer.                    |                                 |
| | Example:                           |                           |                                   |                                 |
| | ACQ:WPOS?                          |                           |                                   |                                 |
| |                                    |                           |                                   |                                 |
| | Query return:                      |                           |                                   |                                 |
| | {write pointer position}           |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:TPOS?**                      |                           | Returns position where            | **rp_AcqGetWritePointerAtTrig** |
| |                                    |                           | trigger event appeared.           |                                 |
| | Example:                           |                           |                                   |                                 |
| | ACQ:TPOS?                          |                           |                                   |                                 |
| |                                    |                           |                                   |                                 |
| | Query return:                      |                           |                                   |                                 |
| | 1234                               |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+

.. tabularcolumns:: |p{28mm}|p{28mm}|p{28mm}|p{28mm}|

+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| SCPI                                 | OPTIONS                   | DESCRIPTION                       | API                             |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| **Data read**                                                                                                                          |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:DATA:UNITS <PAR>**           | <par>={RAW, VOLTS}        | Selects units in which            | **rp_AcqScpiDataUnits**         |
| |                                    |                           | acquired data will be             |                                 |
| | Example:                           | Default: VOLTS            | returned.                         |                                 |
| | ACQ:GET:DATA:UNITS RAW             |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:DATA:FORMAT <PAR>**          | <par>={FLOAT, ASCII}      | Selects format acquired data      | **rp_AcqScpiDataFormat**        |
| |                                    |                           | will be returned.                 |                                 |
| | Example:                           | Default: FLOAT            |                                   |                                 |
| | ACQ:GET:DATA:FORMAT ASCII          |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:SOUR<n>:DATA:STA:END?**      | | <start_pos>             | Read samples from start to        | **rp_AcqGetDataPosRaw**         |
| | **<start_pos>,<end_pos>**          |   ={0,1,...,16384}        | stop position.                    | **rp_AcqGetDataPosV**           |
| |                                    | |                         |                                   |                                 |
| | Example:                           | | <stop_pos>              |                                   |                                 |
| | ACQ:SOUR1:GET:DATA 10,13           |   ={0,1,...16384}         |                                   |                                 |
| |                                    | | stop_pos > start_pos    |                                   |                                 |
| | Query return:                      |                           |                                   |                                 |
| | {123,231,-231}                     |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:SOUR<n>:DATA:STA:N?**        |                           | Read m samples from start         | **rp_AcqGetDataRaw**            |
| | **<start_pos>,<m>**                |                           | position on.                      | **rp_AcqGetDataV**              |
| |                                    |                           |                                   |                                 |
| | Example:                           |                           |                                   |                                 |
| | ACQ:SOUR1:DATA? 10,3               |                           |                                   |                                 |
| |                                    |                           |                                   |                                 |
| | Query return:                      |                           |                                   |                                 |
| | {1.2,3.2,-1.2}                     |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:SOUR<n>:DATA?**              |                           | Read full buf. size starting      | | **rp_AcqGetOldestDataRaw**    |
| |                                    |                           | from oldest sample in             | | **rp_AcqGetOldestDataV**      |
| | Example:                           |                           | buffer (this is first sample      | |                               |
| | ACQ:SOUR2:DATA?                    |                           | after trigger delay). Trigger     | | size=buf_size!                |
| |                                    |                           | delay by default is set to        |                                 |
| | Query return:                      |                           | zero (in samples or in            |                                 |
| | {1.2,3.2,...,-1.2}                 |                           | seconds). If trigger delay is     |                                 |
| |                                    |                           | set to zero it will read full     |                                 |
| |                                    |                           | buf. size starting from           |                                 |
| |                                    |                           | trigger.                          |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:SOUR<n>:DATA:OLD:N?<m>**     |                           | Read m samples after              | **rp_AcqGetOldestDataRaw**      |
| |                                    |                           | trigger delay, starting from      | **rp_AcqGetOldestDataV**        |
| | Example:                           |                           | oldest sample in buffer (this     |                                 |
| | ACQ:SOUR2:DATA:OLD? 3              |                           | is first sample after trigger     |                                 |
| |                                    |                           | delay). Trigger delay by          |                                 |
| | Query return:                      |                           | default is set to zero (in        |                                 |
| | {1.2,3.2,-1.2}                     |                           | samples or in seconds). If        |                                 |
|                                      |                           | trigger delay is set to zero it   |                                 |
|                                      |                           | will read m samples starting      |                                 |
|                                      |                           | from trigger.                     |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:SOUR<n>:DATA:LAT:N?<m>**     |                           | Read m samples before             | **rp_AcqGetLatestDataRaw**      |
| |                                    |                           | trigger delay. Trigger delay      | **rp_AcqGetLatestDataV**        |
| | Example:                           |                           | by default is set to zero (in     |                                 |
| | ACQ:SOUR1:DATA:LAT? 3              |                           | samples or in seconds). If        |                                 |
| |                                    |                           | trigger delay is set to zero it   |                                 |
| | Query return:                      |                           | will read m samples before        |                                 |
| | {1.2,3.2,-1.2}                     |                           | trigger.                          |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+
| | **ACQ:BUF:SIZE?**                  |                           |  Returns buffer size.             | **rp_AcqGetBufSize**            |
| |                                    |                           |                                   |                                 |
| | Example:                           |                           |                                   |                                 |
| | ACQ:BUF:SIZE?                      |                           |                                   |                                 |
| |                                    |                           |                                   |                                 |
| | Query return:                      |                           |                                   |                                 |
| | 16384                              |                           |                                   |                                 |
+--------------------------------------+---------------------------+-----------------------------------+---------------------------------+ 

    
