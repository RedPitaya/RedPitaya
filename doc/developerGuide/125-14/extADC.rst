External ADC clock
##################

ADC clock can be provided by:
    * On board 125MHz XO (default)
    * From external source / through extension connector :ref:`E2 <E2>` (R25,R26 should be moved to location R23,R24)
    * Directly from FPGA (R25,R26 should be moved to location R27,R28)

Schematic:

.. image:: External_clk.png
    :width:  300px

Top side:

.. image:: External_clock_top.png
    :width:  300px

Bottom side:

.. image:: External_clock_bottom.png
    :width:  300px


.. image:: External_clock_bottom_photo.png
    :width:  300px