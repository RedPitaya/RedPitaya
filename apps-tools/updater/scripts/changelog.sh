#!/bin/bash
wget -O /tmp/changelog "http://downloads.redpitaya.com/downloads/$1" &> /dev/null --tries=5 --timeout=1
cat /tmp/changelog
