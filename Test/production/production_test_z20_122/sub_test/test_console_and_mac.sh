#!/bin/bash

source ./sub_test/common_func.sh
source ./sub_test/default_calibration_values.sh

N_REP=5
RP_MAC_BEGINNING='00:26:32'
STATUS=0

echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#            Check the teminal console functionality                   #\e[0m"
echo -e "\e[94m########################################################################\e[0m"

echo
echo "Checking terminal output - USER: check date update"
echo

for i in $(seq 1 1 $N_REP)
do
    date
    sleep 1
done

echo
echo "Checking terminal input - USER: press ENTER"
read
print_test_ok

echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#               RELIMINARY TEST                                        #\e[0m"
echo -e "\e[94m########################################################################\e[0m"

# check if unit-specific environment variables are already written in the EEPROM
READ_MAC=$( $C_PRINTENV | grep ethaddr= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
READ_NAV=$( $C_PRINTENV | grep nav_code= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
READ_HWREV=$( $C_PRINTENV | grep hw_rev= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
READ_SERIAL=$( $C_PRINTENV | grep serial= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
MAC_BEGIN=${READ_MAC:0:8}
HW_BEGIN=${READ_HWREV:0:11}
PrintToFile "hw_rev" "${READ_HWREV//_/-}"

if [[ "$MAC_BEGIN" != "$RP_MAC_BEGINNING" ]] || [[ "$READ_NAV" == "" ]] || [[ "$HW_BEGIN" == "" ]] || [[ "$READ_SERIAL" == "" ]]
then

    echo
    echo "--- PRELIMINARY TEST 1: DEFAULT environment parameters --- "
    echo 


    # Setting enviroment parameters
    echo
    echo "Setting DEFAULT environment parameters on the external EEPROM...."
    cat /opt/redpitaya/environment_parameters.txt > /sys/devices/soc0/amba/e0004000.i2c/i2c-0/0-0050/eeprom
    echo
    sleep 1


    echo
    echo "--- PRELIMINARY TEST 2: Read QR code --- "
    echo

    # Ask the operator to enter the QR CODE
    echo "USER: enter the RedPitaya QR CODE to terminal:"
    read QR_VAR
    sleep 1

    echo  "MAC address is scaned, press enter 2 times to continue"
    read
    read

    # Check QR CODE length and contained MAC, convert it to uppercase
    QR_LENGTH=$(echo $QR_VAR | wc -m)
    MAC_ADDR=$(echo $QR_VAR  | awk -F'[=&-]' '{print $3}' | tr '[:lower:]' '[:upper:]' )
    MAC_BEGIN=${MAC_ADDR:0:8}

    # Ask the user to confirm the MAC address
    echo "Entered MAC address is $MAC_ADDR, press ENTER to confirm it, N/n to enter it again: "
    read USER_ENTER

    # Verify QR CODE length and contained MAC
    while [[ $QR_LENGTH -lt $MIN_QR_LENGTH ]] || [[ "$MAC_BEGIN" != "$RP_MAC_BEGINNING" ]] || [[ "$USER_ENTER" == "N" ]] || [[ "$USER_ENTER" == "n" ]]
    do
        echo "ERROR: QR CODE is NOT valid, enter it again:"
        read QR_VAR

        QR_LENGTH=$(echo $QR_VAR | wc -m)
        MAC_ADDR=$(echo $QR_VAR  | awk -F'[=&-]' '{print $3}' | tr '[:lower:]' '[:upper:]' )
        MAC_BEGIN=${MAC_ADDR:0:8}

        # Ask the user to confirm the MAC address
        echo
        echo "Entered MAC address is $MAC_ADDR, press ENTER to confirm it, N/n to read it again"
        read  USER_ENTER

        echo
    done

    # Parse also the other QR CODE parameters. HP: they are correct.
    NAV_CODE=$(echo $QR_VAR | awk -F'[=&-]' '{print $5}')
    HW_REV=$(echo $QR_VAR   | awk -F'[=&]' '{print $6}')
    #SERIAL=$(echo $QR_VAR   | awk -F'[=&-]' '{print $0}')
    SERIAL='162400000'

    # Save the environment variables
    echo "Writing ethaddr $MAC_ADDR to EEPROM..."
    $C_SETENV ethaddr $MAC_ADDR > /dev/null 2>&1
    echo "Writing navision code $NAV_CODE to EEPROM..."
    $C_SETENV nav_code $NAV_CODE > /dev/null 2>&1
    echo "Writing hw_rev $HW_REV to EEPROM..."
    $C_SETENV hw_rev $HW_REV > /dev/null 2>&1
    echo "Writing serial $SERIAL to EEPROM..."
    $C_SETENV serial $SERIAL > /dev/null 2>&1


    # Verify wrote parameters
    echo
    echo "Verify wrote parameters..."
    READ_MAC=$( $C_PRINTENV | grep ethaddr= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
    READ_NAV=$( $C_PRINTENV | grep nav_code= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
    READ_HWREV=$( $C_PRINTENV | grep hw_rev= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
    READ_SERIAL=$( $C_PRINTENV | grep serial= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1

    if [[ "$READ_MAC" != "$MAC_ADDR" ]] || [[ "$READ_NAV" != "$NAV_CODE" ]] || [[ "$READ_HWREV" != "$HW_REV" ]] || [[ "$READ_SERIAL" != "$SERIAL" ]]
    then
        echo -n "Parameters are NOT correctly written in the EEPROM "
        print_fail
        sleep 1
    else
        echo "Environment variables of this board are SET TO..."
        echo "-----------------------------------------------"
        echo "EEPROM MAC  $READ_MAC"
        echo "NAV         $READ_NAV"
        echo "HWREV       $READ_HWREV"
        echo "SERIAL      $READ_SERIAL"
        echo "-----------------------------------------------"
        echo
    fi
    getDefCalibValues
    export FACTORY_CAL
    ./sub_test/set_calibration.sh
    ./sub_test/set_calibration_fw.sh
    print_calib
    #In any case, reboot the unit to apply the changes
    echo "Rebooting now to apply Enviroment parameters (MAC address)...."
    echo
    echo "Press enter to continue...."
    read
    reboot
    exit 0
else
echo
echo
echo "--- PRELIMINARY-TESTS   SKIPPED --- "
echo
echo "Environment variables are already written to the EEPROM"
echo "Environment variables of this board are next..."
echo
echo "-----------------------------------------------"
echo "EEPROM MAC  $READ_MAC"
echo "NAV         $READ_NAV"
echo "HWREV       $READ_HWREV"
echo "SERIAL      $READ_SERIAL"
echo "-----------------------------------------------"
echo
                # Set default calibration parameters  if the step above is skiped. This is added here
                # beceause Calibration can/will change calibration parameters and if the board  is set once more troguh testing
                # then if cal parameter are not set back to deafult testing step bellow will return an error.
                # Also, when board is set to new testign calibration should be aslo repeated.
getDefCalibValues
export FACTORY_CAL
./sub_test/set_calibration.sh
./sub_test/set_calibration_fw.sh
if [[ "$?" = '0' ]]
then
    STATUS=1
fi
print_calib
fi
exit $STATUS
