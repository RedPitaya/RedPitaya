#/bin/bash
sigrok-cli -i ./*.sr -P spi:mosi=MOSI:clk=CLK:cs=CS#:cpol=1:cpha=0:cs_polarity=active-low  | grep "^.\{9\}$" > sigrock2.out
tail -n +3 sigrock2.out > sigrock.out
rm sigrock2.out
