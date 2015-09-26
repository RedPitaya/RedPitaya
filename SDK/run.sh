#!/bin/bash

# This is a simple run and compile script. It copies the file 
# argv[0] to a specific Red Pitaya with IP argv[1]. It uses putty-tools,
# installed with install.sh. 

#Global variables definition
IP=$1
REDIRECT='/var/log/sdk_log/debug'
EXECUTABLE=$(echo $2 | sed -e 's/.c//')

usage(){
  echo    "Usage: - Input argument 1: Red Pitaya IP address in form 192.168.1.100."
  echo -e "       - Input argument 2: File to be copied, compiled and ran on a redpitaya system.\n"
  exit 1 
}

#TODO: Add timestamp to redirect output
timeStamp(){
  exit 1  
}

if [ $# -eq 0 ] || [ $# -gt 2 ]
  then
     echo -e "\nWrong argument number!"
     usage
fi

echo -e "\nEXECUTING RED PITAYA RUN SCRIPT..."
mkdir -p include
echo -e "\nCOPYING RED PITAYA INCLUDES..."
sshpass -p root scp -r root@$IP:/opt/redpitaya/lib/librp.so include
sshpass -p root scp -r root@$IP:/opt/redpitaya/include/rp.h include

echo -e "\nCOMPILING SOURCE FILE..."

if [ ! -f $2 ]; then
  echo -e "Invalid second argument. File does not exist!\n"
  exit 1
else
  make clean all OBJECT=$2 TARGET=$EXECUTABLE
fi

#Remount red pitaya file system
#sshpass -p root ssh root@$IP 'mount -o rw, remount /opt/redpitaya'

#Creating log directories
sshpass -p root ssh root@$IP 'mkdir -p /var/log/sdk_log'
sshpass -p root ssh root@$IP 'touch /var/log/sdk_log/debug'

#Copying executable file to red pitaya
echo -e "EXECUTING REMOTE FILE..."
echo -e "OUTPUT: \n----------"
sshpass -p root scp $PWD/$EXECUTABLE root@$IP:/tmp
sshpass -p root ssh root@$IP '/tmp/'$EXECUTABLE' | tee '$REDIRECT

#make clean
echo -e "\n----------\nREMOVING ARTIFACTS..."
make clean TARGET=$EXECUTABLE
