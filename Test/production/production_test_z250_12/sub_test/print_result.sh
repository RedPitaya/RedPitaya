#!/bin/bash
source ./sub_test/common_func.sh


echo
echo
echo
echo
echo
echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#            Test result                                               #\e[0m"
echo -e "\e[94m########################################################################\e[0m"
echo
echo "            Summary of test results"
echo
PrintBackLog
echo
echo "Final log:"
CombineLogVar
echo $LOG_VAR
CheckTestPass
