Red Pitaya OS
=============

 - All info is on our `GitHub repository <https://github.com/RedPitaya/RedPitaya>`_
 - More about Red Pitaya OS releases

.. overview
.. --------

.. how to compile
.. --------------

More about Red Pitaya OS releases
---------------------------------

Release notes 0.96-76
^^^^^^^^^^^^^^^^^^^^^

**Logic analyzer:**
    - all trigger inputs are now listed in trigger settings menu
    - UART data decoder was fixed
    - now working also from Firefox browser and on iPhone or iPad
**Oscilloscope:**
    - added export of data into .jpg or .csv format
    - reset button for time delay
**Spectrum analyzer:**
    - improved resolution when spectrum is zoomed in

    
    
Release notes 0.96-20   
^^^^^^^^^^^^^^^^^^^^^^

**New applications:**
    - `Logic analyzer <http://store.redpitaya.com/logic-analyzer.html>`_  application - 125MS/s logic analyzer with 
      I2C, SPI and UART protocol encoder   
    - Network manager - enables user to manage all network settings via WEB interface   

**Spectrum analyser:**
    - hold MIN and MAX features added   
    - export of data into .jpg or .csv format   

**General system improvements:**
    - RP OS is now running Ubuntu 16.04 LTS (GNU/Linux 4.4.0-xilinx armv7l)   
    - security (ssh keys are generated on first reboot, access point is not set by default anymore)   
    - `more <https://github.com/RedPitaya/RedPitaya/blob/master/CHANGELOG.md>`_
    
Release notes 0.95-1
^^^^^^^^^^^^^^^^^^^^
    
**New applications:**
    - `LCR meter application <http://store.redpitaya.com/lcr-meter.html>`_ - accurately measures capacitors, inductors
      and resistors using the test frequencies of 100Hz, 1 kHz, 10kHz and 100kHz.    

**Application improvements:**

#. SCPI server:    
    - can now be started directly from WEB browser via Remote control app    
#. Oscilloscope:    
    - navigation was improved (time offset can now be set by mouse dragging, t/div can be changed by scrolling)    
    - user is now able to retrieve signal acquired into ADC buffer even if acquisition is stopped    
    - stability of data transfer was improved together with network performance indication     
    - gain setting is now remembered when quitting app    
    - trigger status is now properly updated at high frequencies    
    - we fixed few normal trigger issues 

#. Spectrum analyser:
    - acq. algorithm fix that fixes spectrum issues
#. General system improvements:
    - discovery service & market access problems are now solved
    - user is now able to upgrade Red Pitaya OS directly from the WEB browser - rewriting complete SD card image is no longer necessary 
    - Red Pitaya desktop was improved so it is more simple to use and also provides
    - better support for tablets and smartphones
    - browser detection was add that suggest user to install or upgrade to recommended version
    - online/offline detection was added so that user knows if Red Pitaya can access the internet / if it is possible use some features that requires internet connection
    - switching between contributed and official applications doesn’t causes problems anymore
    - system crashes reports with debug information can now be sent to Red Pitaya team on user request
    - Feedback app - enables user to send feedback our support with attached information about Red Pitaya OS, computer OS & browser
    - Analytics is sent to Red Pitaya team on user request in order to track system events that can help improve user experience 

**Other:**
    - access to wiki page was added, but the main news is that we also updated wiki documentation
