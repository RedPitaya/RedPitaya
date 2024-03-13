# Beta version (1.04-30)

- Burst generation mode added to WEB oscilloscope app
- SCPI now supports commands to communicate over I2C, UART and SPI bus available on - Red Pitaya ext. connector
- Added support for RTL8188EU chipset wifi dongles (e.g. Edimax EW-7811Un V2)
- Other bug fixes and improvements
    - SA cursors step issues fixed
    - burst mode issues fixed
    - SCPI examples and doc. updates

# Stable version (1.04-27)

- Spectrum analyzer got new features (improved spectrum resolution, added more windowing functions, units, scaling's and more)
- SCPI server speed was improved
- Signal generator freq. sweep support added
- Bode Analyzer gain & phase calculation algorithms have been improved on speed and precision
- other general improvements & bug fixes
    - added more acquisition decimation options
    - fixed trigger bug for osc. that appeared at very slow signals
    - fixed averaging after signal decimation
    - OS updater notifications
# Stable version (1.04-21)

- Streaming application added

# Stable version (1.04-19)

- Watchdog was implemented for Linux and WEB applications. In case of fatal error or lost connection device will always return to its previous operation state. 
- Open source FPGA code can now be compiled with Vivado 2020.1
- New OS available notification was implemented into WEB interface
other general improvements and updates
- Added application for calibrating RP
 
# Stable version (1.03-1)

General improvements:
- Access point working with official WIFI dongle
- Client WIFI mode fixed
- Trigger issues fixed
- Broken acquired signal issues fixed
- Improved UI of the oscilloscope and spectrum analyzer
- Shutdown button added to WEB interface
- Improved console applications. Added calibration support.
- Other stability improvements & bug fixes

# Beta version (1.00-26)

- General improvements and bugfixing
- SCPI server support added

# Beta version (0.99-19)

- Initial OS release for 250-12
