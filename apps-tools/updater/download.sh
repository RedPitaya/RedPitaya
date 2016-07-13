#!/bin/bash

#max_major=0
#max_minor=0
#max_hash=''
#
#wget -O download.html http://downloads.redpitaya.com/downloads/ &> /dev/null
#
#IFS=$'\n'
#for d in `grep -Po 'ecosystem.+\.zip"' download.html | sed -e 's/ecosystem-0.\([0-9]\+\)-\([0-9]\+\)-\([A-Za-z0-9]\+\).*/\1 \2 \3/g'`; do
#	major=`echo $d | awk '{print $1}'`
#	minor=`echo $d | awk '{print $2}'`
#	hash=`echo $d | awk '{print $3}'`
#
#	if (($major > $max_major || ($major == $max_major && $minor > $max_minor))); then
#		max_major=$major
#		max_minor=$minor
#		max_hash=$hash
#	fi
#done
#grep "ecosystem-0.$max_major-$max_minor-$max_hash.zip" download.html | awk '{print $5}'
#wget -O /tmp/build/build.zip "http://downloads.redpitaya.com/downloads/ecosystem-0.$max_major-$max_minor-$max_hash.zip" &> /dev/null
killall wget &> /dev/null
rm -rf /tmp/build
mkdir -p /tmp/build
rm -rf /tmp/build/*
wget -O /tmp/build/build.zip "http://downloads.redpitaya.com/downloads/0.96/$1" &> /dev/null
echo "OK"
