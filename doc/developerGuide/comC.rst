####################################
Compiling and running C applications
####################################

**Compiling and running on STEMlab board**

When compiling on the target no special preparations are needed. A native toolchain is available directly on the
Debian system.

First connect to your board over :ref:`SSH <ssh>` (replace the IP, the default password is `root`).

.. code-block:: shell-session

    ssh root@192.168.0.100

Now on the target, make a clone of the Red Pitaya Git repository and enter the project directory.

.. code-block:: shell-session

    git clone https://github.com/RedPitaya/RedPitaya.git
    cd RedPitaya

To compile one example just use the source file name without the `.c` extension.

.. code-block:: shell-session

    cd Examples/C
    make digital_led_blink

Applications based on the API require a specific FPGA image to be loaded:

.. code-block:: shell-session

    cat /opt/redpitaya/fpga/fpga_0.94.bit > /dev/xdevcfg

Execute the application. The path to Red Pitaya shared libraries must be provided explicitly. Some applications run in 
a continuous loop, press `CTRL+C` to stop them.
    
.. code-block:: shell-session
    
    LD_LIBRARY_PATH=/opt/redpitaya/lib ./digital_led_blink


More examples about how to control STEMlab using APIs can be found :ref:`here <examples>`.
    
