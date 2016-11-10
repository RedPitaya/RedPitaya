Applications 
############

.. TODO zamenjaj linke z vsebino

`Oscilloscope & Signal Generator <http://redpitaya.com/apps/oscilloscope/>`_
******************************************************************************

`Spectrum Analyzer <http://redpitaya.com/apps/spectrum-analyzer/>`_
******************************************************************************

`Bode Analyzer <http://redpitaya.com/apps/bode-analyzer/>`_
******************************************************************************

`Logic Analyzer <http://redpitaya.com/apps/logic-analyzer/>`_
******************************************************************************

`LCR meter <http://redpitaya.com/apps/lcr-meter/>`_
******************************************************************************

`Marketplace applications <http://redpitaya.com/marketplace-applications/>`_
******************************************************************************


..  (slike, opis, primer uporabe)

.. osciloscope in signal generator isti tekst
.. TODO http://redpitaya.com/apps/oscilloscope/
    dodaj tabelo specsov
    
.. logic analyzer 
.. itd.

.. Bode analyzer
.. *************
.. 
.. Bode analyzer enables measurements of Amplitude and Phase response of the desired [tippy title=”DUT”]Device Under Test[/tippy].
.. The measurements are not in real time.
.. Frequency range and number of steps are adjustable.
.. How to connect DUT to the Red Pitaya when using Bode analyser is shown in picture below.
.. 
.. .. image:: Bode_analyzer_connections.png
.. 
.. Frequency Response analyzer
.. ***************************
.. 
.. Frequency response analyzer enables measurements of frequency amplitude response of desired DUT (Device Under Test).
.. The measurements of frequency response are in range from 0Hz to 60MHz.
.. Measurements are in real time and the frequency range is NOT adjustable.
.. Measurement can be done for each channel independently, i.e it enables simultaneously measurements of two DUTs.
.. How to connect DUT to the Red Pitaya when using Frequency Response analyser is shown in picture below.
.. 
.. .. image:: Frequency_response_analyzer_connections.png
.. 
.. Impedance analyzer – LCR meter
.. ******************************
.. 
.. .. image:: LCR_range.png
.. 
.. Frequency response analyzer enables measurements of frequency amplitude response of desired DUT (Device Under Test).
.. The measurements of frequency response are in range from 0Hz to 60MHz.
.. Measurements are in real time and the frequency range is NOT adjustable.
.. Measurement can be done for each channel independently, i.e it enables simultaneously measurements of two DUTs.
.. How to connect DUT to the Red Pitaya when using Frequency Response analyser is shown in picture below.
.. 
.. .. image:: E_module_connection.png
.. 
.. On pictures below are shown comparison measurements of the selected DUT. Measurements are taken with Red Pitaya and 
.. Keysight precision LCR meter. From this plots you can extract basic Red Pitaya accuracy. Notice Red Pitaya LCR meter / Impedance analyzer are not certificated for certain accuracy or range.
.. 
.. .. image:: LCR_100R.png
..    :width: 45%
.. .. image:: LCR_100K.png
..    :width: 45%
.. .. image:: LCR_1M.png
..    :width: 45%
..    
.. Impedance analyzer application can be used without LCR Extension module using manual setting of shunt resistor. This option is described below. Notice that you will need to change “C_cable” parameter in the code when using your setup.
.. 
.. .. image:: Impedance_analyzer_manaul_R_Shunt.png
.. 
.. 
.. Manually downloading and installing free applications
.. *****************************************************
.. 
.. If you have problems with installing free applications via Bazaar web page or your Red Pitaya doesn’t have an internet access, here are the instructions on how to install free applications manually.
.. 
..     1. Download zip folder of the desired application
..     #. Unzip application folder
..     #. Insert SD card in to your PC, navigate to the “www/apps” folder
..     #. Copy unziped application folder to the “apps” folder
..     
..     
.. .. image:: www_folder.png
.. .. image:: apps_folder.png
.. .. image:: freq_folder.png
