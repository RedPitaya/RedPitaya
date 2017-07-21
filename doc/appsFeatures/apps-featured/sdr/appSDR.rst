SDR transceiver
###############

What is in the box 
******************

The following accessories and materials are included with your STEMlab SDR transceiver kit.

	* C25 160-10 10W module
	* DC power cord with Anderson Power Pole™ connector
	* 4 x SMA cable for connecting C25 module with STEMlab 125-14 and antenna   
	* impedance adapter

.. _Hercules: https://www.hercules.com/uk/leisure-controllers/bdd/p/248/djcontrol-instinct-s-series/

Other additional requirements
*****************************

In addition to the supplied accessories, software and cables supplied with STEMlab SDR transceiver kit, you will need to provide the following:

	* An **HF-Antenna** or dummy load with BNC
	* good RF **ground**	
	* A stabilized DC 13.8 VDC, 3A **Power Supply**
	* DJControl Instinct S Series available from Hercules_ or other midi controller.

SDR application requirements:

	* Personal computer (PC) running Windows 7 or later. Either 32 or 64-bit operating systems are supported. 

Start using STEMlab as Radio Station - SDR transceiver
******************************************************

Connecting the cables
---------------------

.. image :: 16_RedPitaya_Combo2.jpg
   :alt: icon
   :align: center

1. connect Tx of SDR transciver module to Red Pitaya OUT1
2. connect Rx of SDR ransciver module to Red Pitaya IN1 (notice this cable has a transformer)
3. connect control cable from SDR transceiver to Red Pitaya

identify pin with arrow and connect the cable as on the image bellow.

.. image :: 18_RedPitaya_Close.jpg
   :alt: icon
   :align: center
   
4. Make sure jumper is set as shown on image above.
5. Make sure your SD card is inserted and was preinstalled with latest Red Pitaya OS

   - `Red Pitaya OS <http://redpitaya.readthedocs.io/en/latest/quickStart/SDcard/SDcard.html>`_

6. Connect ethernet cable
7. Connect Red Pitaya to power supply 5V 2A
8. Connect antenna

.. note::
	
	STEMlab SDR transceiver module should be powered by DC 13.8V Power Supply that can provide at least 3 A of constant power. 
	Make sure that is turned off and then use DC power cord with Anderson Power Pole™ connector **(9)** to connect it with module. 
	RED wire is positive (+) while BLACK wire is negative (-), double check to not mix the colours or polarity! 
	Don’t turn on the power supply yet.

9. Connect SDR transceiver to 13.8V 3A power supply

10. Turn on power supply

11. Put STEMlab in SDR mode:

	* Make sure your computer is connected to same local area network as STEMlab 
	* Open your WEB browser and connect to your STEMlab (http://redpitaya.readthedocs.io/en/latest/quickStart/first.html)

STEMlab is now ready to connet with Power SDR.

.. tip::
	Optionaly you can connect MIDI controller to your PC. MIDI controller can be used to control radio software parameters like frequency with physical knobs.


Power SDR installation and SDR configuration
********************************************

.. _here: http://downloads.redpitaya.com/hamlab/powersdr/Setup_PowerSDR_Charly_25_HAMlab_STEMlab_Edition.exe

Click here_ to download Power SDR installation package.

1. Start the installation by double clicking on the Setup_PowerSDR_STEMlab_HAMlab_Edition.exe file.

	.. image :: PowerSDRinstallation1.PNG
		:align: center

2. If you are asked for extended user access rights during the installation click Yes! Running installer with administration rights will work as well. 
	
	.. image :: PowerSDRinstallation2.png
		:scale: 70%
   		:align: center
		
On Windows 10 you might get warning of Unknown Publisher you can procede with installation by clicking on "more info" and then "Run anyway".
 
	.. image:: PowerSDRinstallation3.PNG
		:scale: 75 %
   		:align: center
	
	.. image:: PowerSDRinstallation4.PNG
		:scale: 75 %
   		:align: center
	

3. Follow the instructions of the setup routine and accept the license agreements if asked for.

	.. image:: Capture1.PNG
		:scale: 75 %
   		:align: center

	.. image:: Capture2.PNG
		:scale: 75 %
   		:align: center
		
	.. image:: Capture3.PNG
		:scale: 75 %
   		:align: center

	.. image:: Capture4.PNG
		:scale: 75 %
   		:align: center

	.. image:: Capture5.PNG
		:scale: 75 %
   		:align: center

	.. image:: Capture6.PNG
		:scale: 75 %
   		:align: center

	.. image:: Capture7.PNG
		:scale: 75 %
   		:align: center

	.. image:: Capture8.PNG
		:scale: 75 %
		:align: center

4. At the end of the installation you are asked if you want to run PowerSDR software immediately, feel free to do so.

	.. image:: Capture9.PNG
		:scale: 75 %
   		:align: center

5. PowerSDR software will start with the calculation of the FFT wisdom file, **which will take a while** depending on the CPU power of your computer. This is only done once, even after updating the software to a new version in the future:

	.. image:: Capture10.PNG
		:scale: 75 %
   		:align: center

6. After starting the PowerSDR software you will be led through the PowerSDR software specific setup wizard which lets you configure the software to use it with your STEMlab. Pick the HAMlab/STEMlab radio model.

	.. image:: Capture11.PNG
		:scale: 75 %
   		:align: center

7. Select the region where you are using your STEMlab, this is important due to the different frequency ranges your are allowed to transmit in the different countries all over the world:

	.. image:: Capture12.PNG
		:scale: 75 %
   		:align: center

8. Your initial setup is completed click finish.

	.. image:: Capture13.PNG
		:scale: 75 %
   		:align: center

9. Click Power to connect Power SDR with STEMlab. On the screen the input singnal should appear.

	.. image:: Capture20.PNG
		:scale: 75 %
   		:align: center

