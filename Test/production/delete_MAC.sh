#!/bin/bash

PRINTENV="fw_printenv"
SETENV="fw_setenv"


    NAV_CODE=0
    HW_REV=0
    SERIAL=0
    MAC_ADDR=88:88:88:88:88:88

    # Save the environment variables
    echo
    echo "Deleting MAC ethernet address in the EEPROM..."
    echo
    $SETENV ethaddr $MAC_ADDR > /dev/null 2>&1

    echo "Deleting NAV. CODE ethernet address in the EEPROM..."
    echo
    $SETENV nav_code $NAV_CODE > /dev/null 2>&1
    echo "Deleting HW REV. ethernet address in the EEPROM..."
    echo
    $SETENV hw_rev $HW_REV > /dev/null 2>&1

    echo "Deleting SERIAL address in the EEPROM..."
    echo
    $SETENV serial $SERIAL > /dev/null 2>&1

    # Verify wrote parameters
    echo
    echo
    echo "Verify  deleting ..."
    echo
    READ_MAC=$( $PRINTENV | grep ethaddr= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
    READ_NAV=$( $PRINTENV | grep nav_code= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
    READ_HWREV=$( $PRINTENV | grep hw_rev= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
    READ_SERIAL=$( $PRINTENV | grep serial= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1

    if [[ "$READ_MAC" == "$MAC_ADDR" ]] || [[ "$READ_NAV" == "$NAV_CODE" ]] || [[ "$READ_HWREV" == "$HW_REV" ]] || [[ "$READ_SERIAL" == "$SERIAL" ]]
    then
  	    echo "------------------------------------------------"
        echo "Enviroment Parameters in the EEPROM are DELETED"
        echo "------------------------------------------------"
        echo
        echo
        echo "DELETING STATUS OK..."
        echo "-----------------------------------------------"
        echo "EEPROM MAC  $READ_MAC"
        echo "NAV         $READ_NAV"
        echo "HWREV       $READ_HWREV"
        echo "SERIAL      $READ_SERIAL"
        echo "-----------------------------------------------"
        echo
    else
        echo "------------------------------------------------"
        echo "Not able to DELETE Enviroment Parameters..."
        echo "------------------------------------------------"
        echo
        echo
        echo "DELETING STATUS FAILD..."
        echo
        echo "Enviroment Parameters are next"
        echo
        echo "-----------------------------------------------"
        echo "EEPROM MAC  $READ_MAC"
        echo "NAV         $READ_NAV"
        echo "HWREV       $READ_HWREV"
        echo "SERIAL      $READ_SERIAL"
        echo "-----------------------------------------------"
        echo
    fi

  exit
