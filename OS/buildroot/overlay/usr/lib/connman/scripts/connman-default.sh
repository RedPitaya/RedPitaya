#!/bin/sh

LOG=/var/log/connman
PATH=$PATH:/opt/sbin

echo -n "`date`: " >> $LOG
discovery 2>&1 >> $LOG

