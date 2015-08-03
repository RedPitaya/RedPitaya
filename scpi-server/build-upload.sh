#!/bin/bash

./build.sh

sshpass -p 'root' scp scpi-server root@192.168.128.1:/root
sshpass -p 'root' scp 3rdparty/libs/scpi-parser/libscpi/dist/libscpi.so root@192.168.128.1:/usr/lib/
sshpass -p 'root' scp ../api/rpbase/src/librp.so root@192.168.128.1:/usr/lib/
