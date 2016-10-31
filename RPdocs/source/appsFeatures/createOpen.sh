#!/bin/bash

FILENAME="$1.rst"
PARAGRAPH=$2
size=${#PARAGRAPH}
line=`printf "#"'%.s' $(eval "echo {1.."$(($size))"}")`

echo "created file $FILENAME "
echo "paragraph $2, of length $size added"
echo $2    >  $FILENAME
echo $line >> $FILENAME
echo ""    >> $FILENAME

echo "oppening file $FILENAME... "
kate $FILENAME
