#/bin/bash
sigrok-cli -i ./*.sr -P spi:mosi=1:clk=2:cs=0:cpol=1:cpha=0:cs_polarity=active-low:bitorder=msb-first | grep "^.\{9\}$" > sigrock2.out
awk 'NR % 2 == 0' sigrock2.out  > sigrock.out
rm sigrock2.out

