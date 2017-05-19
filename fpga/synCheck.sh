#!/bin/bash

# comon variables
RPT="SYNTHESIS REPORT: "
REPORT="synReport.txt"

# # # # # # # 
# functions #
# # # # # # #

# initial lines to the report section with description as $1 argument
function startRprt {
  TEXT=$1
  SIZE=${#TEXT}
  LINE=`printf "%.s"'-' $(eval "echo {1.."$((100-$SIZE))"}")`
  
  echo "" >> $REPORT
  echo "${TEXT}${LINE}" >> $REPORT
  echo "" >> $REPORT
  
}

# last lines to the report sectoin
function endRprt {
  LINE=`printf "%.s"'-' $(eval "echo {1.."$((100))"}")`
  
  echo "" >> $REPORT
  echo " ${LINE}" >> $REPORT
  echo "" >> $REPORT
  
}

# test passes
function passed {
  echo "PASSED" >> $REPORT
}

function fileMiss {
  # user notification
  echo "ERROR: file $1 not found!"
  
  # to report file
  echo "ERROR: file $1 not found!" >> $REPORT
}

# # # # # # # # # 
# start report  #
# # # # # # # # # 
echo "# # # # # # # # # # # # # # # # # " > $REPORT
echo "#                               #"  >> $REPORT
echo "# $RPT            #"                >> $REPORT
echo "#                               #"  >> $REPORT
echo "# `date` #  "                       >> $REPORT
echo "#                               #"  >> $REPORT
echo "# # # # # # # # # # # # # # # # # " >> $REPORT
echo ""                                   >> $REPORT

# # # # # # # # # # # # # # # # # # # # # # # #
# find sub-optimal timing INFOs in vivado.log #
# # # # # # # # # # # # # # # # # # # # # # # # 
if [ -f "vivado.log" ]; then 

  GREP='grep -i "sub-optimal" vivado.log | sort | uniq'
  SUBN=$(eval "$GREP | grep -i "sub-optimal" -c")

  if [[ $SUBN != 0 ]]; then
    # user notification
    echo "${RPT} WRNING: $SUBN sub-optimal timings detected!"
    
    # to report file
    startRprt "SUB-OPTIMAL TIMING"
    echo "WRNING: $SUBN sub-optimal timings detected:">> $REPORT
    echo "$(eval ${GREP})" >> $REPORT
    endRprt
  else
    # user notification
    echo "sub-optimal timing test - PASSED"
    
    # to report file
    passed 
  fi
else
  fileMiss "vivado.log"
fi

# # # # # # # # # # # # # # # # # # # # # # # 
# displaies violated timings in report FILE #
# # # # # # # # # # # # # # # # # # # # # # # 

function grepTiming {
  FILE=$1
  
  if [ -f ${FILE} ]; then

    GREP='grep -i "violated" ${FILE}'
    SUBN=$(eval "$GREP | grep -i "violated" -c")
  
    startRprt "${FILE} timing violation test"
    if [[ $SUBN != 0 ]]; then
      # user notification
      echo "${RPT} ERROR: $SUBN timing violations detected!"
      
      # to report file
      echo "$(eval grep -i "violated" ${FILE} -A 4)" >> $REPORT
      else
        # user notification
        echo "timing violations test for ${FILE} - PASSED"
      
        # to report file
        passed 
    fi
    endRprt
  else
    fileMiss ${FILE}
  fi
}

# define report directory
FILEPATH="prj/logic/out/"

# check timing in synthesis, placement, route timing reports
grepTiming ${FILEPATH}post_synth_timing_summary.rpt
grepTiming ${FILEPATH}post_place_timing_summary.rpt 
grepTiming ${FILEPATH}post_route_timing_summary.rpt
