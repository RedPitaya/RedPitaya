.. _clu:

######################
Command line utilities
######################

*********************************
Red Pitaya command line utilities
*********************************

.. Note::
   
    Command line utilities must not be used in parallel with a WEB application.
   
    For correct operation of the acquire tool, it is mandatory that the correct FPGA image is loaded. Please note,
    the some application can change the FPGA image loaded.
    To load the FPGA image open a terminal on the Red Pitaya and execute the following command:
    
    .. code-block:: shell-session

       cat /opt/redpitaya/fpga/fpga_0.94.bit > /dev/xdevcfg



.. contents::
    :local:
    :backlinks: none
    :depth: 1   
    
========================
Signal generator utility
========================

The Red Pitaya signal generator can be controlled through the
`generate <https://github.com/RedPitaya/RedPitaya/tree/master/Test/generate>`_ command line utility.


.. tabs::

   .. group-tab:: OS version 0.99 or older

      .. code-block:: shell-session
    
        redpitaya> generate
        generate version 0.90-299-1278

        Usage: generate   channel amplitude frequency <type>

            channel     Channel to generate signal on [1, 2].
            amplitude   Peak-to-peak signal amplitude in Vpp [0.0 - 2.0].
            frequency   Signal frequency in Hz [0.0 - 6.2e+07].
            type        Signal type [sine, sqr, tri].

   .. group-tab:: OS version 1.00

      .. code-block:: shell-session
    
        redpitaya> generate
        generate version 1.00-35-25a03ad-25a03ad

        Usage: generate channel amplitude frequency <gain> <type> <end frequency> <calib>

            channel         Channel to generate signal on [1, 2].
            amplitude       Peak-to-peak signal amplitude in Vpp [0.0 - 2.0].
            frequency       Signal frequency in Hz [0.00 - 1.2e+08].
            gain            Gain output value [x1, x5] (default value x1).
            type            Signal type [sine, sqr, tri, sweep].
            end frequency   Sweep-to frequency in Hz [0.00 - 1.2e+08].
            calib           Disable calibration [-c]. By default calibration enabled


Performance of signal generator differs from one Red Pitaya model to another, for more
information please refer to :ref:`red pitaya boards comparison <red-pitaya-boards-comparison>`

    
==========================
Signal acquisition utility
==========================

The signal from Red Pitaya can be acquired through the `acquire <https://github.com/RedPitaya/RedPitaya/tree/master/Test/acquire>`_
command line utility. It will return raw samples from the ADC buffer to standard output, with no calibration
compensation. Usage instructions (see Table 8 as well):

.. tabs::

    .. group-tab:: OS version 0.99 or older

        .. code-block:: shell-session

            redpitaya> acquire 
            acquire version 0.90-299-1278

            Usage: acquire  size <dec>

                size     Number of samples to acquire [0 - 16384].
                dec      Decimation [1,8,64,1024,8192,65536] (default=1).
        

        Example (acquire 1024 samples with decimation 8):
    
        .. code-block:: shell-session
            
            redpitaya> acquire 1024 8
                -148     -81
                -143     -84
                -139     -88
                -134     -82
                ...

    .. group-tab:: OS version 1.00

        .. code-block:: shell-session

            redpitaya> acquire 

            Usage: acquire [OPTION]... SIZE <DEC>
                --equalization  -e      Use equalization filter in FPGA (default: disabled).
                --shaping       -s      Use shaping filter in FPGA (default: disabled).
                --atten1=a      -1 a    Use Channel 1 attenuator setting a [1, 20] (default: 1).
                --atten2=a      -2 a    Use Channel 2 attenuator setting a [1, 20] (default: 1).
                --dc=c          -d c    Enable DC mode. Setting c use for channels [1, 2, B(Both channels)]. By default, AC mode is turned on.
                --tr_ch=c       -t c    Enable trigger by channel. Setting c use for channels [1P, 1N, 2P, 2N, EP (external channel), EN (external channel)]. P - positive edge, N -negative edge. By default trigger no set
                --tr_level=c    -l c    Set trigger level (default: 0).
                --version       -v      Print version info.
                --help          -h      Print this message.
                --hex           -x      Print value in hex.
                --volt          -o      Print value in volt.
                --no_reg        -r      Disable load registers config for DAC and ADC.
                --calib         -c      Disable calibration parameters
                SIZE                    Number of samples to acquire [0 - 16384].
                DEC                     Decimation [1,8,64,1024,8192,65536] (default: 1).

        
        Example (acquire 1024 samples with decimation 8, ch1 with at 1:20, results displayed in voltage):

        .. code-block:: shell-session

            root@rp-fffff6:~# acquire 1024 8 -1 20 -o
                -0.175803   0.000977
                0.021975    0.001099
                -0.075693   0.000977
                -0.190453   0.001099
                0.004883    0.001221
                -0.046392   0.001099
                -0.200220   0.000977
                -0.014650   0.001099
                -0.019534   0.001099
                -0.195336   0.000977
                -0.041509   0.001099
                ...
        
Performance of acquisition tool differs from one Red Pitaya model to another, for more
information please refer to :ref:`red pitaya boards comparison <red-pitaya-boards-comparison>`


======================================================
Other useful information related to command line tools
======================================================

.. toctree::
   :maxdepth: 6
   
   clt_other

==========================
Accessing system registers
==========================

The system registers can be accessed through the 
`monitor <https://github.com/RedPitaya/RedPitaya/tree/master/Test/monitor>`_ utility. Usage instructions:
 
.. code-block:: shell-session
    
    redpitaya> monitor 
    monitor version 0.90-299-1278

    Usage:
        read addr: address
        write addr: address value
        read analog mixed signals: -ams
        set slow DAC: -sdac AO0 AO1 AO2 AO3 [V]
        
Example (system register reading):
 
.. code-block:: shell-session
    
    redpitaya> monitor -ams 
    #ID                                                                    Desc                                                                                                                                        Raw                                                                    Val
    0                                                                    Temp(0C-85C)                                                                    a4f                                                                    51.634
    1                                                                    AI0(0-3.5V)                                                                    1                                                                    0.002
    2                                                                    AI1(0-3.5V)                                                                    13                                                                    0.033
    3                                                                    AI2(0-3.5V)                                                                    1                                                                    0.002
    4                                                                    AI3(0-3.5V)                                                                    2                                                                    0.003
    5                                                                    AI4(5V0)                                                                    669                                                                    4.898
    6                                                                    VCCPINT(1V0)                                                                    55c                                                                    1.005
    7                                                                    VCCPAUX(1V8)                                                                    9a9                                                                    1.812
    8                                                                    VCCBRAM(1V0)                                                                    55d                                                                    1.006
    9                                                                    VCCINT(1V0)                                                                    55b                                                                    1.004
    10                                                                    VCCAUX(1V8)                                                                    9ab                                                                    1.813
    11                                                                    VCCDDR(1V5)                                                                    809                                                                    1.507
    12                                                                    AO0(0-1.8V)                                                                    2b0000                                                                    0.496
    13                                                                    AO1(0-1.8V)                                                                    150000                                                                    0.242
    14                                                                    AO2(0-1.8V)                                                                    2b0000                                                                    0.496
    15                                                                    AO3(0-1.8V)                                                                    220000                                                                    0.392

You can find some detailed description of the above mentioned pins `here <https://redpitaya.readthedocs.io/en/latest/developerGuide/125-14/extent.html>`_.
The –ams switch provides access to analog mixed signals including Zynq SoC temperature, auxiliary analog input reading, power supply voltages and configured auxiliary analog output settings. The auxiliary analog outputs can be set through the monitor utility using the –sadc switch:
 
.. code-block:: shell-session
    
   redpitaya> monitor -sdac 0.9 0.8 0.7 0.6

============================================
Monitor utility for accessing FPGA registers
============================================

Red Pitaya signal processing is based on two computational engines: the FPGA and the dual core processor in order to
effectively split the tasks. Most of the high data rate signal processing is implemented within the FPGA building 
blocks. These blocks can be configured parametrically through registers. The FPGA registers are documented in the 
`Red Pitaya HDL memory map <https://redpitaya.readthedocs.io/en/latest/developerGuide/regset.html#red-pitaya-modules>`_
document. The registers can be accessed using the described monitor utility. For example, the following sequence of
monitor commands checks, modifies and verifies the acquisition decimation parameter (at address 0x40100014):
 
.. code-block:: shell-session
    
    redpitaya> monitor 0x40100014 
    0x00000001
    redpitaya> 
    redpitaya> monitor 0x40100014 0x8
    redpitaya> monitor 0x40100014 
    0x00000008
    redpitaya>
    
.. note:: 
    
    The CPU algorithms communicate with FPGA through these registers. Therefore, the user should be aware of a 
    possible interference with Red Pitaya applications, reading or acting upon these same FPGA registers. For simple 
    tasks, however, the monitor utility can be used by high level scripts (Bash, Python, Matlab...) to communicate
    directly with FPGA if necessary.
    
