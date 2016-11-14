*********
LCR meter
*********

.. image:: LCR_range.png

Frequency response analyzer enables measurements of frequency amplitude response of desired DUT (Device Under Test).
The measurements of frequency response are in range from 0Hz to 60MHz.
Measurements are in real time and the frequency range is NOT adjustable.
Measurement can be done for each channel independently, i.e it enables simultaneously measurements of two DUTs.
How to connect DUT to the Red Pitaya when using Frequency Response analyser is shown in picture below.

.. image:: E_module_connection.png

On pictures below are shown comparison measurements of the selected DUT. Measurements are taken with Red Pitaya and 
Keysight precision LCR meter. From this plots you can extract basic Red Pitaya accuracy. Notice Red Pitaya LCR meter / Impedance analyzer are not certificated for certain accuracy or range.

.. image:: LCR_100R.png
   :width: 45%
.. image:: LCR_100K.png
   :width: 45%
.. image:: LCR_1M.png
   :width: 45%
   
Impedance analyzer application can be used without LCR Extension module using manual setting of shunt resistor. This option is described below. Notice that you will need to change “C_cable” parameter in the code when using your setup.

.. image:: Impedance_analyzer_manaul_R_Shunt.png
