Software
========

.. (Alexey)


.. How to connect over SSH or console connection
.. ---------------------------------------------

.. quick start
.. ^^^^^^^^^^^

.. how to start server from WEB interface or manualy from shell
.. ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. how to start controling Red Pitaya using Matlab/Python/LabView/SciLab
.. ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Remote control MATLAB/LABVIEW
-----------------------------

Red Pitaya board can be remotely controlled with Matlab/Python/LabView/SciLab softwares using standardized
`SCPI commands <https://en.wikipedia.org/wiki/Standard_Commands_for_Programmable_Instruments>`_.
    
    - Before running your SCPI code on any of these softwares you need to start Red Pitaya SCPI server program.
    - `How to start SCPI server <http://redpitaya.com/control/?with=matlab>`_.
    - `Examples SCPI programs <google.com>`_

**Notice:** To run LabView examples you need to download them. Download the complete `list of supported SCPI commands
<https://dl.dropboxusercontent.com/s/b51h4hp6nnodf0d/SCPI_commands_beta_release_3_3_3016.pdf>`_ to find out more about
the supported functionalities and corresponding parameters.

.. .. 1. MATLAB
.. .. #. LABVIEW
.. .. #. Python
.. .. #. SciLAB
.. 
.. List of supported SCPI commands
.. ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. 

Visual programming
------------------


Getting started with electronics is way more fun and interesting when you have a load of sensors that you can put to 
good use straight away. Whether you want to measure temperature, vibration, movement -or more- we have developed a 
Sensor Extension Module compatible with Grove modules from 
`Seeed® <http://www.seeedstudio.com/depot/category_products?themes_id=1417>`_

    - `More about Visual Programming          <http://redpitaya.com/visual-programming-interface/>`_.
    - `How to Start is shown here             <https://www.youtube.com/watch?v=V4ZSB8oetDQ>`_
    - `Here are some examples - Buzzer        <https://www.youtube.com/watch?v=tRG_xP_KxlM>`_
    - `Here are some examples - Motion sensor <https://www.youtube.com/watch?v=pcRh5aar5dc>`_

.. quick start 
.. """""""""""
..     1. open visual programming
..     #. add new board
..     #. upload file to RP
..     #. start VP server
..     #. board should be online
..     #. start blink example

learn through examples
----------------------

Write “Hello World” program on Red Pitaya board
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Once you have your Red Pitaya board running you can go and make your own executables without compiling the complete 
Red Pitaya OS. The code should be written in C programming language and compiled according to the following 
instructions. `Red Pitayas available APIs <http://libdoc.redpitaya.com/rp_8h.html>` (build in functions) will enable 
you to make program for controlling fast/slow analog inputs/outputs, digital inputs/outputs, communication (I2C, SPI,
UART) and other.

    - How to make simple program

..     - extension board & sensors explained
..     - blocks and controls described

WEB applications
^^^^^^^^^^^^^^^^

Official applications
"""""""""""""""""""""

Oscilloscope and Signal Generator
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Oscilloscope & signal generator features:
    - Run/stop and auto set functionality
    - Signals position and scale controls
    - Trigger controls (source, level, slope)
    - Trigger modes: auto, normal and single triggering
    - Input calibration wizard
    - Cursors
    - Measurements
    - Math operations
    - Signal generator controls (waveform, amplitude, frequency, phase)

Hardware specifications:

Oscilloscope:

.. TODO (http://wiki.redpitaya.com/index.php?title=Oscilloscope_and_Signal_Generator)

=========================== ==========================
Input channels              2 (SMA connectors)
Bandwidth	                50 MHz
Memory depth                16384 Samples Max.
Sampling Rate	            125 MS/s
Input range                 +/- 1V (LV*) +/- 20V (HV*)
Minimal Voltage Sensitivity	± 0.244 mV / ± 2.44 mV
Pin on Extension Connector
Input coupling	            DC
=========================== ==========================

LV- low voltage jumper settings HV- high voltage jumper setting More about 
`inputs specification <http://wiki.redpitaya.com/index.php?title=Analog_frontend_-_Inputs>`_.

Signal generator:

=================   ==========================
Output channels     2 (SMA connectors)
Bandwidth           50 MHz
Signal buffer       16384 Samples Max.
Sampling Rate       125 MS/s
Output range        +/- 1V
Frequency Range     0 - 50 MHz
External Trigger	Pin on Extension Connector
=================   ==========================


More about `outputs specification <http://wiki.redpitaya.com/index.php?title=Analog_frontend_-_Outputs>`_.


Spectrum analyzer
~~~~~~~~~~~~~~~~~

.. TODO slike (http://wiki.redpitaya.com/index.php?title=Spectrum_analyzer)


This version of Spectrum analyzer app. is based on FFT calculation and it not on sweep mode processing.

Spectrum analyzer features: 

Bandwidth 0 - 62 MHz

Input channels 2 SMA connectors

Dynamic Range - 80dBm

Input noise level < -119 dBm/Hz

ADCs Resolution 14 bit

Input range +/- 1 V (LV) LV- low voltage jumper settings

Input impedance 1 MΩ / 10 pF

Input coupling DC

Spurious frequency components < -90 dBFS Typically

..     1. overview
..     #. seting the environment
..     #. how to create own WEB app
..     #. examples
..     #. how to upload it to market place
.. 
.. Command line tools
.. ^^^^^^^^^^^^^^^^^^
.. generate
.. """"""""
.. acquire
.. """""""
.. 
.. monitor
.. """""""
.. 
.. calibrate
.. """""""""

Command line utilities
----------------------

.. TODO slike (http://wiki.redpitaya.com/index.php?title=Command_line_utilities)

Red pitaya command line utilities
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Note: Command line utilities must not be used in parallel with a WEB application**

Signal generator utility
""""""""""""""""""""""""

The Red Pitaya signal generator can be controlled through the generate command line utility, but be aware it 
interferes with the GUI based Oscilloscope & Generator application. Usage instructions (see Table 7 as well)::

    redpitaya> generate
    generate version 0.90-299-1278

    Usage: generate   channel amplitude frequency <type>

           channel     Channel to generate signal on [1, 2].
           amplitude   Peak-to-peak signal amplitude in Vpp [0.0 - 2.0].
           frequency   Signal frequency in Hz [0.0 - 6.2e+07].
           type        Signal type [sine, sqr, tri].

==========  ======  ============================= ==============================================================================
Name        Type    Range                         Description
==========  ======  ============================= ==============================================================================
channel     int     1 / 2                         Output channel selection
amplitude   float   0 - 2 [V]                     Maximal output signal is 2 V peak to peak
freq        float   0 - 62000000\ :sup:`1`  [Hz]  Frequency can be generated from 0 Hz (DC signal) on*.
<type>      string  sine / sqr / tri              Optional parameter. Signal shape type (sine – sine wave signal, sqr – square 
                                                  signal, tri – triangular signal). If omitted, sine is used.
==========  ======  ============================= ==============================================================================

\ :sup:`1` To generate smooth signals, not exceeding Back-End bandwidth, limitations are:
    - 62 MHz (62000000) for sine wave
    - 10 MHz (10000000) for square and triangular waves

The output can be disabled by setting the amplitude parameter to zero.

Example (2 Vpp square wave signal with 1 MHz on channel 1)::

    redpitaya> generate 1 2 1000000 sqr
    
**Note** that the signal generator output impedance is 50 Ω. If user wants to connect the output of the signal 
generator (OUT1, OUT2) to the Red Pitaya input (IN1, IN2), 50 Ω terminations should be connected at the Red Pitaya 
inputs through the T-type connector.

Signal acquisition utility
""""""""""""""""""""""""""

The signal from Red Pitaya can be acquired through the acquire command line utility. It will return raw samples from 
the ADC buffer to standard output, with no calibration compensation. Usage instructions (see Table 8 as well)::

    redpitaya> acquire 
    acquire version 0.90-299-1278

    Usage: acquire  size <dec>

           size     Number of samples to acquire [0 - 16384].
           dec      Decimation [1,8,64,1024,8192,65536] (default=1).

========== ===== =================    ==============================================================================
Name       Type  Range	              Description
========== ===== =================    ==============================================================================
size       int   0 - 16384	          The number of samples to read.
dec        int	 1, 8, 64, 1024,      Optional parameter. It specifies the decimation factor. If omitted, 1 is used 
                 8192, 16384	      (no decimation). 
========== ===== =================    ==============================================================================

Acquire utility will return the requested number of samples with decimation factor for both input channels (column 1 
= Channel1; column 2 = Channel2).

Example (acquire 1024 samples with decimation 8)::

    redpitaya> acquire 1024 8
    -148     -81
    -143     -84
    -139     -88
    -134     -82
    ...

Saving data buffers
"""""""""""""""""""

It is recommended to use an NFS share to store any temporary data (e.g. the measured signals using the acquire 
utility). Use a standard mount command to mount your NFS share (example)::
    
    redpitaya> mount -o nolock <ip_address>:/<path>  /mnt
    
The /opt file-system on Red Pitaya, representing the SD card, is mounted read-only. To save the data locally on Red 
Pitaya redirect the acquisition to a file in the /tmp directory. The /tmp directory resides in RAM and is therefore 
volatile (clears on reboot). ::  

    redpitaya> acquire 1024 8 > /tmp/my_local_file

Alternatively, save the data directly to the NFS mount point::

    redpitaya> acquire 1024 8 > /mnt/my_remote_file
    
Copying data - Linux users
""""""""""""""""""""""""""

.. http://wiki.redpitaya.com/index.php?title=Command_line_utilities

In case NFS share is not available, you can use secure copy::

    redpitaya> scp my_local_file <user>@<destination_ip>:/<path_to_directory>/

Alternatively Linux users can use graphical SCP/SFTP clients, such as Nautilus for example (explorer window). 
To access the address line, type [CTRL + L] and type in the following URL: sftp://root@<ip_address>

.. TODO Nautilus address bar.png http://wiki.redpitaya.com/index.php?title=Command_line_utilities

Figure: Nautilus URL/address bar.
Type the Red Pitaya password (next Figure). The default Red Pitaya password for the root account is »root«. For 
changing the root password, refer to buildroot configuration - a mechanism for building the Red Pitaya root 
file-system, including the /etc/passwd file hosing the root password.

.. Nautilus password window.png http://wiki.redpitaya.com/index.php?title=Command_line_utilities

After logging in, the main screen will show the directory content of Red Pitaya’s root filesystem. Navigate to select 
your stored data and use the intuitive copy-paste and drag & drop principles to manipulate the files on Red Pitaya 
(see next Figure).

.. TODO Nautilus root fs.png http://wiki.redpitaya.com/index.php?title=Command_line_utilities

Copying data - Windows users
""""""""""""""""""""""""""""

Windows users should use an SCP client such as WinSCP. Download and install it, following its installation 
instructions. To log in to Red Pitaya, see example screen in next Figure.

.. TODO WinSCP login screen.png http://wiki.redpitaya.com/index.php?title=Command_line_utilities

Figure: WinSCP login screen.
After logging in, the main screen will show the content of the Red Pitaya root filesystem. Navigate to select your
stored data and use the intuitive copy-paste and drag & drop principles to manipulate the files on Red Pitaya (see 
next Figure).

.. TODO WinSCP directory content.png http://wiki.redpitaya.com/index.php?title=Command_line_utilities

Figure: Directory content on Red Pitaya.
Select the destination (local) directory to save the data file to (see next Figure).

.. TODO WinSCP filesave.png http://wiki.redpitaya.com/index.php?title=Command_line_utilities
