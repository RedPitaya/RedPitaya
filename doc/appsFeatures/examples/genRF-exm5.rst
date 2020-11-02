Generate two synchronous signal
###############################

.. http://blog.redpitaya.com/examples-new/generate-signal-on-fast-analog-outputs-with-external-triggering/

Description
***********

This example shows how to program Red Pitaya to generate two synchronous analog signals. Voltage and frequency ranges depends on Red Pitaya model.


Required hardware
*****************

    - Red Pitaya device

.. image:: output_y49qDi.gif

Code - MATLAB®
**************

The code is written in MATLAB. In the code we use SCPI commands and TCP/IP communication. Copy code to MATLAB editor
and press run.

.. code-block:: matlab

    %% Define Red Pitaya as TCP/IP object
    clc
    clear all
    close all

    IP= '192.168.178.56';            % Input IP of your Red Pitaya...
    port = 5000;
    tcpipObj=tcpip(IP, port);


    %% Open connection with your Red Pitaya
 
    fopen(tcpipObj);
    tcpipObj.Terminator = 'CR/LF';
    
    fprintf(tcpipObj,'GEN:RST');
    fprintf(tcpipObj,'SOUR1:FUNC SINE');       % Set function of output signal
                                               % {sine, square, triangle, sawu,sawd, pwm}
    fprintf(tcpipObj,'SOUR1:FREQ:FIX 2000');   % Set frequency of output signal
    fprintf(tcpipObj,'SOUR1:VOLT 1');          % Set amplitude of output signal

    fprintf(tcpipObj,'SOUR2:FUNC SINE');       % Set function of output signal                                       
                                               % {sine, square, triangle, sawu,sawd, pwm}
    fprintf(tcpipObj,'SOUR2:FREQ:FIX 2000');   % Set frequency of output signal
    fprintf(tcpipObj,'SOUR2:VOLT 1');          % Set amplitude of output signal
        
    fprintf(tcpipObj,'OUTPUT:STATE ON');       % Start two channels simultaneously
    
    %% Close connection with Red Pitaya
    
    fclose(tcpipObj);

Code - C
********

.. note::

    C code examples don't require the use of the SCPI server, we have included them here to demonstrate how the same functionality can be achieved with different programming languages. 
    Instructions on how to compile the code are here -> `link <https://redpitaya.readthedocs.io/en/latest/developerGuide/comC.html>`_

.. code-block:: c

    /* Red Pitaya external trigger pulse generation Example */

    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>

    #include "rp.h"


    int main(int argc, char **argv){

        /* Print error, if rp_Init() function failed */
        if(rp_Init() != RP_OK){
            fprintf(stderr, "Rp api init failed!\n");
        }
        
        rp_GenWaveform(RP_CH_1, RP_WAVEFORM_SINE);
        rp_GenFreq(RP_CH_1, 2000);
        rp_GenAmp(RP_CH_1, 1);

        rp_GenWaveform(RP_CH_2, RP_WAVEFORM_SINE);
        rp_GenFreq(RP_CH_2, 2000);
        rp_GenAmp(RP_CH_2, 1);

        rp_GenOutEnableSync(true);

        /* Release rp resources */
        rp_Release();

        return 0;
    }

Code - Python
*************

.. code-block:: python

    #!/usr/bin/python

    import sys
    import redpitaya_scpi as scpi

    rp_s = scpi.scpi("192.168.1.17")

    wave_form = 'sine'
    freq = 2000
    ampl = 1

    rp_s.tx_txt('GEN:RST')
    rp_s.tx_txt('SOUR1:FUNC ' + str(wave_form).upper())
    rp_s.tx_txt('SOUR1:FREQ:FIX ' + str(freq))
    rp_s.tx_txt('SOUR1:VOLT ' + str(ampl))
    rp_s.tx_txt('SOUR2:FUNC ' + str(wave_form).upper())
    rp_s.tx_txt('SOUR2:FREQ:FIX ' + str(freq))
    rp_s.tx_txt('SOUR2:VOLT ' + str(ampl))
    rp_s.tx_txt('OUTPUT:STATE ON')



