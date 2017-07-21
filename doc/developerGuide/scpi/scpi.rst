####
SCPI
####

SCPI standard is defined here.

********************
Install dependencies
********************

Python

**************
Implementation
**************

*************
SCPI commands
*************

The next subsystems are available:

Oscilloscope:

Generator:
``:OUTPUT[<n>]``
``:SOURCE[<n>]``

The value of ``n`` selects one of the ``N`` oscilloscope channels.
The indexing starts at ``1`` and ends at ``N``.
The available options for ``n`` are ``1`` or ``2``.

    {.pattern = "SOURce#:RESET",                              .callback = rpscpi_gen_reset,},
    {.pattern = "SOURce#:START",                              .callback = rpscpi_gen_start,},
    {.pattern = "SOURce#:STOP",                               .callback = rpscpi_gen_stop,},
    {.pattern = "SOURce#:TRIGger",                            .callback = rpscpi_gen_trigger,},

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
since internaly all values are rounded.

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
since internaly all values are rounded.

A new frequency is applied immediately.

----------
Parameters
----------

+---------+-------------+------------+---------+--------------+
| Name    | Type        | Range      | Default | Default unit |
+=========+=============+============+=========+==============+
| <phase> | real number | 0° to 360° | 1 kHz   | degree (°)   |
+---------+-------------+------------+---------+--------------+

The unit (degree symbol) should not be provided,
other units are not supported yet.
Negative values and values greater then 360° are properly wrapped.


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
since internaly all values are rounded.

A new phase is only applied after the generator is triggered again.

----------
Parameters
----------

+-------------+----------------------+------------+---------+--------------+
| Name        | Type                 | Range      | Default | Default unit |
+=============+======================+============+=========+==============+
| <frequency> | Positive real number | 0° to 360° | 1 kHz   | degree (°)   |
+-------------+----------------------+------------+---------+--------------+

The unit (degree symbol) should not be provided,
other units are not supported yet.
Negative values and values greater then 360° are properly wrapped.


    {.pattern = "[SOURce#]:FUNCtion[:SHAPe]",                 .callback = rpscpi_gen_set_waveform_tag,},
    {.pattern = "[SOURce#]:FUNCtion[:SHAPe]?",                .callback = rpscpi_gen_get_waveform_tag,},
    {.pattern = "[SOURce#]:VOLTage[:IMMediate][:AMPlitude]",  .callback = rpscpi_gen_set_amplitude,},
    {.pattern = "[SOURce#]:VOLTage[:IMMediate][:AMPlitude]?", .callback = rpscpi_gen_get_amplitude,},
    {.pattern = "[SOURce#]:VOLTage[:IMMediate]:OFFSet",       .callback = rpscpi_gen_set_offset,},
    {.pattern = "[SOURce#]:VOLTage[:IMMediate]:OFFSet?",      .callback = rpscpi_gen_get_offset,},
//  {.pattern = "[SOURce#]:PHASe:INITiate",                   .callback = RP_GenPhaseInit,},
//    {.pattern = "[SOURce#]:TRACe:DATA[:DATA]",                .callback = RP_GenArbitraryWaveForm,},
//    {.pattern = "[SOURce#]:TRACe:DATA[:DATA]?",               .callback = RP_GenArbitraryWaveFormQ,},
    {.pattern = "[SOURce#]:BURSt[:MODE]",                     .callback = rpscpi_gen_set_burst_mode,},
    {.pattern = "[SOURce#]:BURSt[:MODE]?",                    .callback = rpscpi_gen_get_burst_mode,},
    {.pattern = "[SOURce#]:BURSt:DATA:REPetitions",           .callback = rpscpi_gen_set_data_repetitions,},
    {.pattern = "[SOURce#]:BURSt:DATA:REPetitions?",          .callback = rpscpi_gen_get_data_repetitions,},
    {.pattern = "[SOURce#]:BURSt:DATA:LENgth",                .callback = rpscpi_gen_set_data_length,},
    {.pattern = "[SOURce#]:BURSt:DATA:LENgth?",               .callback = rpscpi_gen_get_data_length,},
    {.pattern = "[SOURce#]:BURSt:PERiod:LENgth",              .callback = rpscpi_gen_set_period_length,},
    {.pattern = "[SOURce#]:BURSt:PERiod:LENgth?",             .callback = rpscpi_gen_get_period_length,},
    {.pattern = "[SOURce#]:BURSt:PERiod:NUMber",              .callback = rpscpi_gen_set_period_number,},
    {.pattern = "[SOURce#]:BURSt:PERiod:NUMber?",             .callback = rpscpi_gen_get_period_number,},
