#!/bin/bash
source ./sub_test/common_func.sh

CombineLogVarLocal

###############################################################################
# LOGGING TEST TO SD CARD
###############################################################################
echo
echo "------------Logging test result to SD card------------------------------"
echo
# SAVE RESULTS INTO THE SD-CARD
# Remount the SD card with R&W rigths
mount -o remount,rw $G_SD_CARD_PATH

# Log information to the log file
echo "      Test data logging on the SD card was successful"
echo $LOG_VAR >> "$G_SD_CARD_PATH/$G_LOG_FILENAME"
RPLight7
# Remount the SD card with Read only rigths
mount -o remount,ro $G_SD_CARD_PATH
echo
echo "------------------------------------------------------------------------"
echo


###############################################################################
# LOGGING TEST TO LOCAL PC
###############################################################################
# Create the mounting point folder and mount the device
N_PING_PKG=5
RES=$(ping "$G_LOCAL_SERVER_IP" -c "$N_PING_PKG" | grep 'transmitted' | awk '{print $4}' ) > /dev/null

if [[ $G_SAVE_TO_PC == 1 ]]
then
if [[ "$RES" == "$N_PING_PKG" ]]
then
    echo "-------------Logging test statistics to PRODUCTION PC-------------------"
    echo
    #echo 'sometext' | ssh zumy@192.168.178.123 "cat >> /home/zumy/Desktop/Test_LOGS/manuf_test.log"
    echo $LOG_VAR | sshpass -p "$G_LOCAL_SERVER_PASS" ssh -oStrictHostKeyChecking=no -oUserKnownHostsFile=/dev/null $G_LOCAL_USER@$G_LOCAL_SERVER_IP "cat >> $G_LOCAL_SERVER_DIR/$G_LOG_FILENAME"
    if [[ $? = 255 ]]
    then
      echo -n "      Test data logging on the local PC "
      print_fail
    else
      echo -n "      Test data logging on the local PC "
      print_ok
    fi
    else
    echo -n "      Not possible to log test statistics to local PC "
    print_fail
fi
echo
echo
echo "------------------------------------------------------------------------"
echo
echo
fi

CombineLogVar

###############################################################################
# LOGGING TEST RESULTS TO SERVER
###############################################################################
echo
echo "------------Logging test result to the Red Pitaya d.d MAIN server---------"
echo
echo "      Checking if board is online and server is available.."
echo
CURL_RSP="$(curl -Is http://production.redpitaya.com | head -1)"
#echo $CURL_RSP
echo
if [ `echo $CURL_RSP | grep -c "HTTP/1.1" ` -gt 0 ]
then
echo -n "      Board is online & server is available. "
print_ok
else
echo -n "      Board is offline or server is not available. Logging test result to the Red Pitaya d.d MAIN server FAILD!!! "
print_fail
exit
fi
echo
echo "      Sending test record data to server..."
# Prepare LOG_VAR data
LOG_VAR="${LOG_VAR// /_}"
#echo $LOG_VAR
echo
MD5IN="$LOG_VAR $G_PASS"
#echo $MD5IN
MD5OUT=`echo -n $MD5IN | md5sum | awk '{print $1}'`
#echo $MD5OUT
echo
URL="http://production.redpitaya.com/Sync/LogUpdate?test_rec_data=$LOG_VAR"
echo $URL
echo
#URL1="www.redpitaya.com"
#echo $URL1
#curl $URL1
CURL_RSP="$(curl $URL)"
if [ `echo $CURL_RSP | grep -c "Stored successfully" ` -gt 0 ]
then
echo
echo -n "      Test record was successfully added to production database. "
print_ok
echo
elif [ `echo $CURL_RSP | grep -c "This entry already exists in DB" ` -gt 0 ]
then
echo
echo
echo -n "      This board  (combination of MAC & DNA) already exists in production database with PASS(0x1ff = 111111111) status!!! "
print_fail
echo
echo
else
echo
echo -n "      No response from SERVER!!! "
print_fail
echo
fi
echo
echo
echo "---------------------------------------------------------------------------------------------------------------------------------------------"
