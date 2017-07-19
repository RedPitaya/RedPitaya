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

``:OUTPut[<n>][:STATe] ON|OFF``
``:OUTPut[<n>][:STATe]?``

-----------
Description
-----------

Enable/disable the generator output where ``n`` is the index (1,2).
Query returns generator output enable status in the same format as parameters.

----------
Parameters
----------

+------+------+---------+---------+ 
| Name | Type | Range   | Default | 
+======+======+=========+=========+ 
|      | Bool | ON\|OFF | OFF     | 
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
|      | Mnemonic | PERiodic\|BURSt | PERiodic | 
+------+----------+-----------------+----------+ 


====================================
``[:SOURce[<n>]]:FREQuency[:FIXed]``
====================================

-------
Syntax:
-------

``[:SOURce[<n>]]:FREQuency[:FIXed] <frequency>|MINimum|MAXimum``
``[:SOURce[<n>]]:FREQuency[:FIXed]?``

-----------
Description
-----------

Select either periodic or burst mode for generator.
Query returns generator mode in the same format as the parameters. 

----------
Parameters
----------

+-------------+----------------------+-----------------+----------+ 
| Name        | Type                 | Range           | Default  | 
+=============+======================+=================+==========+ 
| <frequency> | Positive real number | PERiodic\|BURSt | 1 kHz    | 
+-------------+----------------------+-----------------+----------+ 


    {.pattern = "[SOURce#]:FREQuency[:FIXed]",                .callback = rpscpi_gen_set_frequency,},
    {.pattern = "[SOURce#]:FREQuency[:FIXed]?",               .callback = rpscpi_gen_get_frequency,},
    {.pattern = "[SOURce#]:FUNCtion[:SHAPe]",                 .callback = rpscpi_gen_set_waveform_tag,},
//    {.pattern = "[SOURce#]:FUNCtion[:SHAPe]?",                .callback = rpscpi_gen_get_waveform_tag,},
    {.pattern = "[SOURce#]:VOLTage[:IMMediate][:AMPlitude]",  .callback = rpscpi_gen_set_amplitude,},
    {.pattern = "[SOURce#]:VOLTage[:IMMediate][:AMPlitude]?", .callback = rpscpi_gen_get_amplitude,},
    {.pattern = "[SOURce#]:VOLTage[:IMMediate]:OFFSet",       .callback = rpscpi_gen_set_offset,},
    {.pattern = "[SOURce#]:VOLTage[:IMMediate]:OFFSet?",      .callback = rpscpi_gen_get_offset,},
    {.pattern = "[SOURce#]:PHASe[:ADJust]",                   .callback = rpscpi_gen_set_phase,},
    {.pattern = "[SOURce#]:PHASe[:ADJust]?",                  .callback = rpscpi_gen_get_phase,},
//  {.pattern = "[SOURce#]:PHASe:INITiate",                   .callback = RP_GenPhaseInit,},
//    {.pattern = "[SOURce#]:FUNCtion[:SQUare]:DCYCle",         .callback = RP_GenDutyCycle,},
//    {.pattern = "[SOURce#]:FUNCtion[:SQUare]:DCYCle?",        .callback = RP_GenDutyCycleQ,},
//    {.pattern = "[SOURce#]:FUNCtion[:TRIangle]:DCYCle",       .callback = RP_GenDutyCycle,},
//    {.pattern = "[SOURce#]:FUNCtion[:TRIangle]:DCYCle?",      .callback = RP_GenDutyCycleQ,},
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
