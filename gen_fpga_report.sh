#!/bin/bash

PROJECT_FPGA=$1
REPORT_FILE=$2
MODE=$3

if [[ "$MODE" == "FPGA" ]]
then

echo "<section name=\"FPGA/$PROJECT_FPGA\" fontcolor=\"\">" > $REPORT_FILE

for f in build/fpga/$PROJECT_FPGA/*; do
    if [ -d "$f" ]; then
        DIR_NAME=$(basename $f)
        INFO=$(cat $f/git_info.txt)
        COMMIT=$(awk 'NR==2 {print $2}' $f/git_info.txt)
        echo "<field name=\"$DIR_NAME\" titlecolor=\"blue\" value=\"$COMMIT\" detailcolor=\"black\" href=\"https://gitlab.redpitaya.com/redpitaya-3.0/redpitaya-fpga/-/commit/$COMMIT\"> <![CDATA[ $INFO ]]> </field>" >> $REPORT_FILE
    fi
done

echo "</section>" >> $REPORT_FILE
fi

if [[ "$MODE" == "KERNEL" ]]
then
    echo "<section name=\"Kernel\" fontcolor=\"\">" > $REPORT_FILE
    BRANCH=$(git show -s --pretty=%D HEAD | awk '{gsub("origin/",""); print $2}')
    LOG=$(git log -n 1)
    COMMIT=$(git log -n -1 | awk 'NR==1 {print $2}')
    echo "<field name=\"$BRANCH\" titlecolor=\"blue\" value=\"https://gitlab.redpitaya.com/redpitaya-3.0/redpitaya-public/-/commit/$COMMIT\" detailcolor=\"black\" href=\"https://gitlab.redpitaya.com/redpitaya-3.0/redpitaya-public/-/commit/$COMMIT\"> <![CDATA[ $LOG ]]> </field>" >> $REPORT_FILE
    echo "</section>" >> $REPORT_FILE

fi


if [[ "$MODE" == "ECO" ]]
then
    echo "<section name=\"Ecosystem\" fontcolor=\"\">" > $REPORT_FILE
    BRANCH=$(git branch --contains HEAD)
    LOG=$(git log -n 1)
    COMMIT=$(git log -n -1 | awk 'NR==1 {print $2}')
    echo "<field name=\"$BRANCH\" titlecolor=\"blue\" value=\"https://gitlab.redpitaya.com/redpitaya-3.0/redpitaya-public/-/commit/$COMMIT\" detailcolor=\"black\" href=\"https://gitlab.redpitaya.com/redpitaya-3.0/redpitaya-public/-/commit/$COMMIT\"> <![CDATA[ $LOG ]]> </field>" >> $REPORT_FILE
    echo "</section>" >> $REPORT_FILE
fi
