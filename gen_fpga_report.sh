#!/bin/bash

PROJECT_FPGA=$1
REPORT_FILE=$2

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
