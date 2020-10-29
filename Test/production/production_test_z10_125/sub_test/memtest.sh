#!/bin/bash
source ./sub_test/common_func.sh

###############################################################################
# Memory test
###############################################################################

echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#            Test of RAM                                               #\e[0m"
echo -e "\e[94m########################################################################\e[0m"

STATUS=0
echo
echo "Start memory test..."
echo
MEMORY_SIZE=170

$C_MEM_TEST_TOOL $MEMORY_SIZE 1

echo -n "Memory testing completed "

if [[ $? != 0 ]]
then
    STATUS=1
    print_test_fail
else
    print_test_ok
fi



exit $STATUS