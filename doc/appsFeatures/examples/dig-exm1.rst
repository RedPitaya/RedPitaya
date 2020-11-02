.. _blink:

#####
Blink
#####

.. http://blog.redpitaya.com/examples-new/blink/

***********
Description
***********

This example shows how to control one of the Red Pitaya on board LEDs and make it blink.

*****************
Required hardware
*****************

    - Red Pitaya device

.. image:: output_y49qDi.gif

***************
Code - MATLAB ®
***************

The code is written in MATLAB. In the code we use SCPI commands and TCP/IP communication. Copy code from below to 
MATLAB editor, save project and press run.

.. code-block:: matlab

    %% Define Red Pitaya as TCP/IP object
            
    IP= '192.168.178.56';           % Input IP of your Red Pitaya...
    port = 5000;
    tcpipObj=tcpip(IP, port);

    %% Open connection with your Red Pitaya

    fopen(tcpipObj);
    tcpipObj.Terminator = 'CR/LF';

    %% Send SCPI command to Red Pitaya to turn ON LED1

    fprintf(tcpipObj,'DIG:PIN LED1,1');

    pause(5)                         % Set time of LED ON

    %% Send SCPI command to Red Pitaya to turn OFF LED1

    fprintf(tcpipObj,'DIG:PIN LED1,0');

    %% Close connection with Red Pitaya

    fclose(tcpipObj);

    
********
Code - C
********

.. note::

    C code examples don't require the use of the SCPI server, we have included them here to demonstrate how the same functionality can be achieved with different programming languages. 
    Instructions on how to compile the code are here -> `link <https://redpitaya.readthedocs.io/en/latest/developerGuide/comC.html>`_

.. code-block:: c

    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>

    #include "rp.h"

    int main (int argc, char **argv) {
        int unsigned period = 1000000; // uS
        int unsigned led;

        // index of blinking LED can be provided as an argument
        if (argc > 1) {
            led = atoi(argv[1]);
        // otherwise LED 0 will blink
        } else {
            led = 0;
        }
        printf("Blinking LED[%u]\n", led);
        led += RP_LED0;

        // Initialization of API
        if (rp_Init() != RP_OK) {
            fprintf(stderr, "Red Pitaya API init failed!\n");
            return EXIT_FAILURE;
        }

        int unsigned retries = 1000;
        while (retries--){
            rp_DpinSetState(led, RP_HIGH);
            usleep(period/2);
            rp_DpinSetState(led, RP_LOW);
            usleep(period/2);
        }

        // Releasing resources
        rp_Release();

        return EXIT_SUCCESS;
    }

*************
Code - Python
*************

.. code-block:: python

    #!/usr/bin/python

    import sys
    import time
    import redpitaya_scpi as scpi

    rp_s = scpi.scpi(sys.argv[1])

    if (len(sys.argv) > 2):
    led = int(sys.argv[2])
    else:
    led = 0

    print ("Blinking LED["+str(led)+"]")

    period = 1 # seconds

    while 1:
        time.sleep(period/2.0)
        rp_s.tx_txt('DIG:PIN LED' + str(led) + ',' + str(1))
        time.sleep(period/2.0)
        rp_s.tx_txt('DIG:PIN LED' + str(led) + ',' + str(0))


*************
Code - Scilab
*************

.. code-block:: scilab

    clc

    // Load SOCKET Toolbox. Steps 7&8
    exec(SCI+'contribsocket_toolbox_2.0.1loader.sce'); 
    SOCKET_init();

    IP= '192.168.128.1';
    port = 5000;
    tcpipObj='RedPitaya';

    SOCKET_open(tcpipObj,IP,port);

    SOCKET_write(tcpipObj,'DIG:PIN LED1,1');
    xpause(5*1E+6)
    SOCKET_write(tcpipObj,'DIG:PIN LED1,0');

    SOCKET_close(tcpipObj);

**************
Code - LabVIEW
**************

.. image:: Blink_LV.png


`Download <https://downloads.redpitaya.com/downloads/Clients/labview/Blink.vi>`_

