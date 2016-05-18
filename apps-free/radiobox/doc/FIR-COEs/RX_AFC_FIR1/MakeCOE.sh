#!/bin/sh

OUTNAME=rb_fir3_200k_to_200k_24c_17i16_35o.coe

# first clear file
cat 0_*  >$OUTNAME
cat 1_* >>$OUTNAME

for file in 2_* 3_* 4_* 5_*; do
	echo "," >>$OUTNAME
	cat $file >>$OUTNAME
done

cat *Footer* >>$OUTNAME

