#!/bin/bash
wget -O download.html http://downloads.redpitaya.com/downloads/ &> /dev/null

IFS=$'\n'
for l in `cat download.html`; do
	echo $l | grep -Po 'ecosystem-.+\.zip"' | tr -d '"'
	if [[ $l == *"ecosystem"* ]]
		then echo $l | awk '{ print $5}'
	fi
done
