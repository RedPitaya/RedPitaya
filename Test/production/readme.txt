# How to set start of the script after logg in

# go to cd /root
# open bashrc 
# nano .bashrc
# at the end of the script inser line 
# bash /opt/redpitaya/production_testing_script.sh


# SSH key generation for data logging on production pc
# Start redpitaya, open terminal input commands bellow. (for each SD card used)
# ssh-keygen -t rsa
# ssh-copy-id redpitaya@192.168.178.100 

 # delete_MAC.sh is used to delete enviroment parameters.
 # For deleting Enviroment parameters Test Pin needs to be grounded.

 #  external_eeprom_data.txt is file whcih needs to be written on hte external eeprom 
 # (This  eeprom is placed on the TEST board.) For writting on it jumper J3 must be set. 
 # How to is described here: external_eeprom_rw_commands.txt


 # You need to install bc calculator on once you start OS 


 # For logging on the producton PC RSA keys needs to be set on each SD card (after OS is started)

 # https://www.digitalocean.com/community/tutorials/how-to-set-up-ssh-keys--2

 #  For the USB flash drive test you need to have  file  with name  usb_device_testfile.txt

 # Test data are logged on USB drive, SD card , local PC and SERVER.
 # File is named manuf_file.log

   
		
# calibration_script.sh 	
# delete_MAC.sh 	
# production_testing_script.sh 

# needs to be on the /opt/redpitaya  folder.


# On first booting process default os enviroment parameters will be used after that 
# the Red Pitaya default parameters should be written to the onboard eeprom. 
# Default parameters are in enviroment_paramters.txt file. This file is "cat" to the eeprom at the start 
# of the testing script. 
# If tihs step is not performed the new booting proccess will fail. (stops at zynq-uboot).
# Here you can manually set enviroment parametres to enable new boot. 
# use "setenv" in "saveenv" commands to change "modeboot"  parameter

#zyng-uboot> printenv
#bootcmd 
#should be set to
#zyng-uboot> setenv bootcmd run $modeboot
#zyng-uboot> saveenv
  