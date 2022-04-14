#!/bin/bash
timeout 3 wget -O /tmp/requirements.txt "https://downloads.redpitaya.com/downloads/$1/requirements.txt" &> /dev/null

IFS=$'\n'
for l in `cat /tmp/requirements.txt`; do
	NAME=$(echo $l  | cut -d';' -f1)
	VER=$(echo $l  | cut -d';' -f2)
	if [[ $2 == $NAME ]]
		then echo $VER
	fi
done
