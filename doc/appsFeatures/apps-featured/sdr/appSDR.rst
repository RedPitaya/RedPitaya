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
	* A stabilized DC 13.8 VDC, 4A **Power Supply**
	* DJControl Instinct S Series available from Hercules_ or other midi controller.

SDR application requirements:

	* Personal computer (PC) running Windows 7 or later. Either 32 or 64-bit operating systems are supported. 

Start using STEMlab as Radio Station - SDR transceiver
******************************************************

Connecting the cables
---------------------

1. Set IN1 jumper to the middle of STEMlab 125-14 
2. Mount impedance adapter to IN1, connect the other side with C25 module
3. Connect OUT1 with C25 module
4. Connect Rx bypass filters 
5. Connect antenna cable 
6. Connect control cable 
7. Connect power cable to DC 13.8V power supply
8. Turn ON / power your STEMlab board

.. note::
	
	STEMlab SDR transceiver module should be powered by DC 13.8V Power Supply that can provide at least 4 A of constant power. 
	Make sure that is turned off and then use DC power cord with Anderson Power Pole™ connector **(1)** to connect it with module. 
	RED wire is positive (+) while BLACK wire is negative (-), double check to not mix the colours or polarity! 
	Don’t turn on the power supply yet.

9. Put STEMlab in SDR mode:

	* Make sure your computer is connected to same local area network as STEMlab 
	* Open your WEB browser and connect to your STEMlab (http://redpitaya.readthedocs.io/en/latest/quickStart/first.html)
	* Start SDR HPSDR web application first. 

.. image :: hpsdr_icon.png
   :alt: icon
   :align: center
   
Click on the SDR icon in order to put STEMlab into SDR mode. While web application is running STEMlab will be in SDR mode and you can connect to it with PowerSDR software to use it as radio.
   
.. image :: webapp.png   

STEMlab is now ready to connet with Power SDR.

.. note:: 

	Exiting this SDR WEB application will close the connection to Power SDR.

.. tip::
	Optionaly you can connect MIDI controller to your PC. MIDI controller can be used to control radio software parameters like frequency with physical knobs.
	

Power SDR installation and SDR configuration
********************************************

.. _here: http://downloads.redpitaya.com/hamlab/powersdr/Setup_PowerSDR_Charly_25_HAMlab_Edition.exe

Click here_ to download Power SDR installation package.

1. Start the installation by double clicking on the Setup_PowerSDR_STEMlab_HAMlab_Edition.exe file.

	.. image :: PowerSDRinstallation1.PNG

2. If you are asked for extended user access rights during the installation click Yes! Running installer with administration rights will work as well. 
	
	.. image :: PowerSDRinstallation2.png
		:scale: 70%
		
On Windows 10 you might get warning of Unknown Publisher you can procede with installation by clicking on "more info" and then "Run anyway".
 
	.. image:: PowerSDRinstallation3.PNG
		:scale: 75 %
	
	.. image:: PowerSDRinstallation4.PNG
		:scale: 75 %
	

3. Follow the instructions of the setup routine and accept the license agreements if asked for.


4. At the end of the installation you are asked if you want to run PowerSDR software immediately, feel free to do so.


5. After starting the PowerSDR software the first time you will be led through the PowerSDR software specific setup wizard which lets you configure the software to use it with your STEMlab.

So please choose STEMlab SDR transceiver kit as your radio model:

.. image :: powersdrsetup01.jpg

6. Confirm the RedPitaya as HPSDR hardware (currently there is no other type of hardware available for the Hamlab).

.. image :: powersdrsetup02.jpg

7. Select the region where you are using your STEMlab, this is important due to the different frequency ranges your are allowed to transmit in the different countries all over the world:

.. image :: powersdrsetup03.jpg

8. Your initial setup is completed:

.. image :: powersdrsetup04.jpg

9.  After clicking the Finish button PowerSDR software will start with the calculation of the FFT wisdom file, **which will take a while** depending on the CPU power of your computer.
This is only done once, even after updating the software to a new version in the future:

.. image :: powersdrsetup05.jpg

10. When all calculations are done, PowerSDR software will come up with the main window:

.. image :: powersdrsetup06.jpg

11. Click Power to connect Power SDR with STEMlab. On the screen the input singnal should appear.

.. image :: SDRconnectepower.PNG


