#!/bin/bash

src/build.sh

sshpass -p 'root' scp testApp root@192.168.178.56:/root/
sshpass -p 'root' scp src/librp.so root@192.168.178.56:/usr/lib/
sshpass -p 'root' ssh root@192.168.178.56 chmod +x testApp
