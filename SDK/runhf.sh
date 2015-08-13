#!/bin/bash

# This is a simple run and compile script. It copies the file 
# argv[0] to a specific Red Pitaya with IP argv[1]. It uses putty-tools,
# installed with install.sh. 

#Global variables definition
CROSS_COMPILE='arm-linux-gnueabi-gcc '
CFLAGS='-g -std=gnu99 -Wall -Werror'
RP_LIB_INCLUDE='-L /lib/lirp.so -lm -lpthread -lrp'
IP=$1
REDIRECT=' >> /var/log/sdk_log/debug'
#Remote execute command
REMOTE_COMPILE_CMD=$CROSS_COMPILE$CFLAGS$RP_LIB_INCLUDE$REDIRECT


usage(){
  echo    "Usage: - Input argument 1: Red Pitaya IP address in form 192.168.1.100."
  echo -e "       - Input argument 2: File to be copied, compiled and ran on a redpitaya system.\n"
  exit 1 
}

if [ $# -eq 0 ] || [ $# -gt 2 ]
  then
     echo -e "\nWrong argument number!"
     usage
fi 

echo -e "\nEXECUTING RED PITAYA RUN SCRIPT..."
sshpass -p root ssh -o StrictHostKeyChecking=no root@$1
sshpass -p root ssh -o root scp $PWD/$2 root@$1:/opt/redpitaya
sshpass -p root ssh -o root@$IP '/opt/redpitaya/'$1
#Installing all needed tools on a remote Red Pitaya 
sshpass -p root ssh -o root@$IP 'apt-get install gcc-arm-linux-gnueabi'
sshpass -p root ssh -o root@$IP 'mkdir -p /var/log/sdk_log'
if [ sshpass -p root ssh -o root@$IP '! -f /tmp/log/sdk_log/debug' ]
  then
    sshpass -p root ssh -o root@$IP 'touch /tmp/log/sdk_log/debug'
fi

sshpass -p root ssh -o root@$IP $REMOTE_EXECUTE_COMMAND
#Test everything
