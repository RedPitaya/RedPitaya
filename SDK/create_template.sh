#!/bin/bash

# This is a simple script for generating api red pitaya frameworks
# The idea is for the user to create a simple red pitaya
# api framework instead ready for programming.

#Global variable definition

usage(){
  echo -e "Usage: - Input argument 1: Framework .c file name.\n"
  exit 1
}

if [ $# -eq 0 ] || [ $# -gt 1 ]
  then
    echo -e "\nWrong argument number!"
   usage
fi

echo -e "GENERATING FRAMEWORK...\n"
if [ ! -f $1 ]
  then 
    touch $1
  else
    echo "File exists!"
   exit 1
fi

#Framework generating system
{ echo -e "#include <stdio.h>";
  echo -e '#include "rp.h"\n';
  echo -e "int main(int argc, char **argv){\n";
  echo -e "   /* Init the red pitaya resources */";
  echo -e "   if(rp_Init() != RP_OK){";
  echo -e "      return -1;\n   }\n";
  echo -e "   /* -- Code goes here -- */\n";
  echo -e "   /* Release the red pitaya module resources */";
  echo -e "   rp_Release();\n";
  echo -e "   return RP_OK;\n}";
} >> $1
