####
SCPI
####

This is a new SCPI server based on API1.

TODO:
- link to the standard, 

********************
Install dependencies
********************

For running SCPI client examples written in ``Python``
the next dependencies have to be installed:

.. code-block:: shell-session

   sudo apt install python3-pip
   pip3 install --upgrade pip
   sudo pip3 install pyvisa
   sudo pip3 install pyvisa-py

Since ``PyVISA-py`` is not fully featured and buggy, you might prefer using NI-VISA.
Only RPM based distributions are supported,
for those follow the installation instructions.

For Debian/Ubuntu based distributions try the next instructions:

1. Download the `NI-VISA ISO file <http://www.ni.com/download/ni-visa-17.0/6700/en/>`_ and mount it.

.. code-block:: shell-session

   sudo apt-get install rpm
   sudo ./INSTALL --accept-license --no-install-labview-support --nodeps
   sudo updateNIDrivers
   sudo visaconf

http://forums.ni.com/t5/Linux-Users/Using-NI-VISA-with-Arch-Linux-or-Ubuntu-14-04/gpm-p/3462361#M2287


******************
Running the server
******************

The SCPI server can be started as a ``systemd`` service.

.. code-block:: shell-session

   systemctl start scpi

To start the server at boot, the service should be enabled.

.. code-block:: shell-session

   systemctl enable scpi


**************
Implementation
**************


********
Building
********

For developers who wish to modify the SCPI server and/or the underlying API,
the best option is to compile directly on the Red Pitaya board.
After cloning the Git repository and changing into the directory do:

.. code-block:: shell-session

   meson builddir --prefix /opt/redpitaya --buildtype release
   cd builddir
   ninja

The server can be installed using the nect commands:

.. code-block:: shell-session

   rw
   ninja install

And run as a systemd service:

.. code-block:: shell-session

   systemctl restart scpi.service

Alternatively the server can be run directly from the build directory:

.. code-block:: shell-session

   LD_LIBRARY_PATH=api1:subprojects/scpi-parser-redpitaya-2017/libscpi/ scpi/scpi


*************
SCPI commands
*************

The next subsystems are available:

Oscilloscope:
``ACQ:SOURCE[<n>]``

Generator:
``:SOURCE[<n>]``
``:OUTPUT[<n>]``

The value of ``n`` selects one of the ``N`` oscilloscope channels.
The indexing starts at ``1`` and ends at ``N``.
The available options for ``n`` are ``1`` or ``2``.


========================
``:OUTPut[<n>][:STATe]``
========================

-------
Syntax:
-------

``:OUTPut[<n>][:STATe] ON|OFF|0|1``
``:OUTPut[<n>][:STATe]?``

-----------
Description
-----------

Enable/disable the generator output where ``n`` is the index (1,2).
Query returns generator output enable status as a number.

----------
Parameters
----------

+------+------+---------+---------+
| Name | Type | Range   | Default |
+======+======+=========+=========+
|      | bool | ON\|OFF | OFF     |
+------+------+---------+---------+


=====================
``:SOURce[<n>]:MODE``
=====================

-------
Syntax:
-------

``:SOURce[<n>]:MODE PERiodic|BURSt``
``:SOURce[<n>]:MODE?``

-----------
Description
-----------

Select either periodic or burst mode for generator.
Query returns generator mode in the same format as the parameters. 

----------
Parameters
----------

+------+----------+-----------------+----------+
| Name | Type     | Range           | Default  |
+======+==========+=================+==========+
|      | mnemonic | PERiodic\|BURSt | PERiodic |
+------+----------+-----------------+----------+


====================================
``[:SOURce[<n>]]:FREQuency[:FIXed]``
====================================

-------
Syntax:
-------

``[:SOURce[<n>]]:FREQuency[:FIXed] <frequency>``
``[:SOURce[<n>]]:FREQuency[:FIXed]?``

-----------
Description
-----------

Specify signal frequency when generator is in periodic mode.
Query might return a slightly different value,
since internally all values are rounded.

----------
Parameters
----------

+-------------+----------------------+---------------+---------+--------------+
| Name        | Type                 | Range         | Default | Default unit |
+=============+======================+===============+=========+==============+
| <frequency> | positive real number | up to 62.5MHz | 1 kHz   | Hz           |
+-------------+----------------------+---------------+---------+--------------+

If no unit is provided the default is **Hz**,
but units like **kHz** and **MHz** can also be used.


=================================
``[:SOURce[<n>]]:PHASe[:ADJust]``
=================================

-------
Syntax:
-------

``[:SOURce[<n>]]:PHASe[:ADJust] <phase>``
``[:SOURce[<n>]]:PHASe[:ADJust]?``

-----------
Description
-----------

Specify signal phase when generator is in periodic mode.
Query might return a slightly different value,
since internally all values are rounded.

A new phase is only applied after the generator is triggered again.

----------
Parameters
----------

+---------+-------------+------------+---------+--------------+
| Name    | Type        | Range      | Default | Default unit |
+=========+=============+============+=========+==============+
| <phase> | real number | 0° to 360° |    0°   | degree (°)   |
+---------+-------------+------------+---------+--------------+

The unit (degree symbol) should not be provided,
other units are not supported yet.
Negative values and values greater than 360° are properly wrapped.

===============================
``[:SOURce#]:FUNCtion[:SHAPe]``
===============================

-------
Syntax:
-------

``[:SOURce#]:FUNCtion[:SHAPe] SINusoid|SQUare|TRIangle|USER, [<duty_cycle>]``
``[:SOURce#]:FUNCtion[:SHAPe]?``

-----------
Description
-----------

Specify the shape to be loaded into the waveform table.
The ``USER`` shape is ignored, since an arbitrary waveform can be loaded
regardless of the current shape setting.

The ``SQUare`` and the ``TRIangle`` shapes support the ``<duty_cycle>`` parameter.
The ``<duty_cycle>`` parameter is unitless in the range from 0 to 1 by default.
Optional units are ``PCT`` (%) and ``PPM`` (parts per milion).

For ``SQUare`` the waveform is ``1`` for ``<duty_cycle>``\*period
and ``-1`` for the rest.
For ``TRIangle`` the waveform is rising from ``-1`` to ``+1`` for
``<duty_cycle>``\*period and falling toward ``-1`` for the rest.

Query returns waveform shape in the same format as the parameters.

----------
Parameters
----------

+--------------+----------+----------------------------------+----------+--------------+
| Name         | Type     | Range                            | Default  | Default unit |
+==============+==========+==================================+==========+==============+
|              | mnemonic | SINusoid\|SQUare\|TRIangle\|USER |          |              |
+--------------+----------+----------------------------------+----------+--------------+
| <duty_cycle> | float    | 0 to 1 *or* 0PCT to 100PCT       | 0.5      | none         |
+--------------+----------+----------------------------------+----------+--------------+

================================
``[:SOURce#]:TRACe:DATA[:DATA]``
================================

-------
Syntax:
-------

``[:SOURce#]:TRACe:DATA[:DATA] <data>``
``[:SOURce#]:TRACe:DATA[:DATA]? [<len>]``

-----------
Description
-----------

Specify the the arbitrary waveform table.
An arbitrary number (between 1 and table size) of data points
in the normalized range [-1,+1] can be provided.
The number of data points will also initilalize in internal
periodic mode table size register.
For burst mode data length needs to be set separately.

An arbitrary number ``<len>`` (between 1 and table size) of data points
can be requested. If the ``<len>`` parameter is absent,
the entire table will b returned.

----------
Parameters
----------

+--------+-------------+------------+---------+--------------+
| Name   | Type        | Range      | Default | Default unit |
+========+=============+============+=========+==============+
| <data> | float array | -1 to +1   |         | V            |
+--------+-------------+------------+---------+--------------+
| <len>  | integer     | 1 to 16384 | 16384   |              |
+--------+-------------+------------+---------+--------------+


