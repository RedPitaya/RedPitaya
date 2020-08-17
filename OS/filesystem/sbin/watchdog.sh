#!/bin/bash

exec 3>/dev/watchdog
while :
do
  echo 1>&3
  sleep 5
done

