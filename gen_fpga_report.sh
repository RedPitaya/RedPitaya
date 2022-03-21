#!/bin/bash

FPGA_NAME=$1
COMMIT=$2
REPORT_FILE=report.xml

cd fpga/$FPGA_NAME
BRANCH=$(git name-rev $COMMIT)
LOG=$(git log -n 3)
cd ../..

echo "<section name=\"$FPGA_NAME\" fontcolor=\"\">" >> $REPORT_FILE

echo "<field name=\"Branch\" titlecolor=\"\" value=\"$BRANCH\" detailcolor=\"\" href=\"\"> <![CDATA[ $LOG ]]> </field>" >> $REPORT_FILE
 
echo "</section>" >> $REPORT_FILE