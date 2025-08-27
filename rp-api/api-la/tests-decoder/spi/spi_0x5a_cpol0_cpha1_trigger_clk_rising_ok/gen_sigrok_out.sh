#/bin/bash
sigrok-cli -i ./*.sr -P spi:mosi=MOSI:clk=CLK:cs=CS#:cpol=0:cpha=1:cs_polarity=active-low:bitorder=msb-first  | grep "^.\{9\}$" > sigrock2.out
tail -n +4 sigrock2.out > sigrock.out
rm sigrock2.out
