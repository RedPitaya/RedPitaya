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
CheckTestPass

if [[ $STATUS = 1 ]]
then
    echo
    echo -e "\033[92m"
    echo "                  ********************"
    echo "                  ********************"
    echo "                  *                  *"
    echo "                  *                  *"
    echo "                  *    BOARD PASS    *"
    echo "                  *                  *"
    echo "                  *                  *"
    echo "                  ********************"
    echo "                  ********************"
    echo -e "\e[0m"
    echo
    echo "Test status is: $TEST_RES"
    echo "Data logging status is: $(date) "

else
    echo
    echo -e "\033[91m"
    echo "                  *********************"
    echo "                  *********************"
    echo "                  *                   *"
    echo "                  *                   *"
    echo "                  *    BOARD FAILED   *"
    echo "                  *                   *"
    echo "                  *                   *"
    echo "                  *********************"
    echo "                  *********************"
    echo -e "\e[0m"
    echo
    echo "Test status is: $TEST_RES"
    echo "Data logging status is: $(date) "

fi

echo
echo "Final log:"
CombineLogVarLocal
echo $LOG_VAR
