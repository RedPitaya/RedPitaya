#!/bin/bash

make CROSS_COMPILE=arm-linux-gnueabi- clean all

sshpass -p 'root' scp testApp root@192.168.178.56:/root/
sshpass -p 'root' scp librp.so root@192.168.178.56:/usr/lib/

sshpass -p 'root' ssh root@192.168.178.56 chmod +x testApp
