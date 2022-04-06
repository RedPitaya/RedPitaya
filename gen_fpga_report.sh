#!/bin/bash

MODE=$1
FPGA_NAME=$2
COMMIT=$3
REPORT_FILE=report.xml

if [[ "$MODE" == "BRANCH_KERNEL" ]]
then
BRANCH=$(git branch --contains HEAD)
LOG=$(git log -n 1)
echo "<field name=\"Kernel\" titlecolor=\"blue\" value=\"$BRANCH\" detailcolor=\"black\" href=\"$BRANCH\"> <![CDATA[ $LOG ]]> </field>" >> $REPORT_FILE
fi

if [[ "$MODE" == "BRANCH_ECOSYSTEM" ]]
then
BRANCH=$(git branch --contains HEAD)
LOG=$(git log -n 1)
echo "<field name=\"Ecosystem\" titlecolor=\"blue\" value=\"$BRANCH\" detailcolor=\"black\" href=\"$BRANCH\"> <![CDATA[ $LOG ]]> </field>" >> $REPORT_FILE
fi

if [[ "$MODE" == "GEN" ]]
then
cd fpga/$FPGA_NAME
BRANCH=$(git name-rev $COMMIT)
LOG=$(git log -n 1)
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
