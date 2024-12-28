#/bin/bash
sigrok-cli -i ./*.sr -P spi:mosi=MOSI:clk=CLK:cs=CS#:cpol=0:cpha=1:cs_polarity=active-low:bitorder=lsb-first  | grep "^.\{9\}$" > sigrock.out
#tail -n +3 sigrock2.out > sigrock.out
#rm sigrock2.out
