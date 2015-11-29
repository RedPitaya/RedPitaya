This directory contains the Makefiles and the patches used to compile Python for Red Pitaya

Introduction
============

Python is a widely used general-purpose, high-level programming language. It can be compiled and installed on the Red Pitaya board. Using Python on the board, one can run script without using the C langage. Furthermore, mainy libraries are available for Python and can be used directly. 

Installation
============

The easiest way is to use an SD-card image where python is installed. You can download such an image following this `link <http://clade.pierre.free.fr/python-on-red-pitaya/ecosystem-0.92-0-devbuild.zip>`_

You can also build the full redpitaya ecosystem using this fork instead of the official github repo. See for example this `tutorial <http://forum.redpitaya.com/viewtopic.php?t=49>`_


PyRedPitaya package
===================

This package provides a library to access the FPGA registers. This library consist of a C library (libmonitor.c) and a ctypes interface on the Python side. 

An object oriented interface to the different application (scope, generator, PID, AMS, ...) is provided. This interface is implemented using Python properties (see usage below) and can quickly be extended to your own application. 

An rpyc server is used in order to communicate with your computer. The interface is the same on the computer as the one on the board.

Usage
=====

Interactive Python
------------------

Logging onto the redpitaya using ssh, one can start the ipython shell and directly write or test python code. You can acces to the redpitaya interface using the folowing 

.. code ::

    from PyRedPitaya.board import RedPitaya

    redpitaya = RedPitaya()

    print redpitaya.read(0x40000000) # Direct access

    print redpitaya.ams.temp # Read property
    redpitaya.hk.led = 0b10101010 # Write property


Remote access
-------------

You need to install the PyRedPitaya package on your PC as well as Rpyc: 

.. code::

    sudo easy_install PyRedPitaya

On the redpitaya, you have to start the server. Log onto the board using ssh and run : 

..code::

    rpyc_server

On the computer (replace REDPITAYA_IP by the string containing the IP address) : 

.. code::

    from rpyc import connect
    from PyRedPitaya.pc import RedPitaya

    conn = connect(REDPITAYA_IP, port=18861)
    redpitaya = RedPitaya(conn)

    print redpitaya.ams.temp # Read property
    redpitaya.hk.led = 0b10101010 # Write property

    from time import sleep
    from pylab import *

    redpitaya.scope.setup(frequency = 100, trigger_source=1)
    sleep(100E-3)
    plot(redpitaya.scope.times, redpitaya.scope.data_ch1)
    show()

Background script
-----------------

Example to log the FPGA temperature. Copy the following script in a file, make it exacutable and run it in the background. 

.. code::

    #!/usr/bin/python

    from PyRedPitaya.board import RedPitaya
    from time import sleep

    red_pitaya = RedPitaya()

    with open('/tmp/log.txt', 'a') as f:
        while True:
            f.write(str(red_pitaya.ams.temp)+'\n')
            sleep(1)



Example
=======

* led : progress bar using the on board leds.

* rpyc_server : run an rpyc server

* send_temperature_by_email: send the temperature of the FPGA by email. You can use the SMPT and EMAIL environnement variable if you don't want to be prompt for the smtp server address and your email address.

Installation details
====================

Python compilation
------------------

The cross compilation of Python is not straight forward. The compilation of Python is a two step process : the python interpreter is first build and then used to build the full python distribution. When you cross compile python, you need to run the interpreter on the host. The Makefile will first compile python for the host, using the host C compiler. Then it will build a patched version of python for the ARM using the cross-compiler. 

Python depends on many external library that are already install on the RedPitaya (buildroot). The Makefile provides the cross-compiler with the correct directories. In order for Python to fully work on the ARM, you need to first compile the buildroot of the RedPitaya. 

The python binaries as well as all the packages are installed on the SD card under the usr/ directory (mounted on /opt/usr). The rcS file in /opt/etc/init.d/ can be modified to link the python interpreter to /usr/bin/python . 



Non standard packages
---------------------

Other usefull packages are also installed :

* numpy : library used to perform calculation on arrays
* ipython : interactive python
* MyHDL : MyHDL is a library that allow to write and simulate HDL using the python syntax. Furthermore, it can convert MyHDL codes to verilog. MyHDL is used on the RedPitaya board to interact with register. 
* Rpyc : Using Rpyc, one can interact with the onboard python using a remote computer. 

