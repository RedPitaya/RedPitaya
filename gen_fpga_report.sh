#!/bin/bash

MODE=$1
FPGA_NAME=$2
COMMIT=$3
REPORT_FILE=report.xml



if [[ "$MODE" == "GEN" ]]
then
cd fpga/$FPGA_NAME
BRANCH=$(git name-rev $COMMIT)
LOG=$(git log -n 3)
cd ../..
echo "<field name=\"$FPGA_NAME\" titlecolor=\"blue\" value=\"$BRANCH\" detailcolor=\"black\" href=\"$BRANCH\"> <![CDATA[ $LOG ]]> </field>" >> $REPORT_FILE
fi

if [[ "$MODE" == "PACK" ]]
then
mv $REPORT_FILE $REPORT_FILE.fpga.sub.tmp

echo "<section name=\"FPGA\" fontcolor=\"\">" > $REPORT_FILE

cat $REPORT_FILE.fpga.sub.tmp >> $REPORT_FILE
 
echo "</section>" >> $REPORT_FILE

rm $REPORT_FILE.fpga.sub.tmp

fi
