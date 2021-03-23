#!/bin/bash
wget -O /tmp/changelog "http://downloads.redpitaya.com/downloads/$1" &> /dev/null
cat /tmp/changelog
